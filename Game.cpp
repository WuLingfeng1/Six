#include"Game.h"
#include"Chesswoman.h"
#include"vector"
#include <algorithm>
#include <array>
#include <cmath>
#include <random>
#include <QString>
#include <QMessageBox>
#include <QThread>
using std::vector;

namespace {
constexpr int INF = 1000000000;      // 搜索正/负无穷
constexpr int WIN_SCORE = 100000000; // 已成六连（必胜）的评分
constexpr int WIN_THRESHOLD = 80000; // estimate 返回超过此值视为直接杀棋

struct SixWindowGeometry {
    std::array<std::array<int, 6>, 924> windows{};
    std::array<std::vector<int>, 361> incidence;
    SixWindowGeometry() {
        const int directions[4][2] = {{0,1}, {1,0}, {1,1}, {1,-1}};
        int index = 0;
        for (int x = 0; x < 19; ++x) for (int y = 0; y < 19; ++y)
            for (const auto& dir : directions) {
                const int ex = x + 5 * dir[0], ey = y + 5 * dir[1];
                if (ex < 0 || ex >= 19 || ey < 0 || ey >= 19) continue;
                for (int k = 0; k < 6; ++k) {
                    const int point = (x + k * dir[0]) * 19 + y + k * dir[1];
                    windows[index][k] = point;
                    incidence[point].push_back(index);
                }
                ++index;
            }
        Q_ASSERT(index == 924);
    }
};
const SixWindowGeometry& sixWindows() {
    static const SixWindowGeometry geometry;
    return geometry;
}

}

Game::SearchConfig Game::searchConfig;

constexpr int TT_SIZE = 1 << 18;

static std::mt19937& searchRng()
{
    static std::mt19937 rng(std::random_device{}());
    return rng;
}
void Game::seedRandom(unsigned seed)
{
    searchRng() = std::mt19937(seed);
}

struct ZobristTable {
    quint64 table[19][19][3];
    quint64 side;
    ZobristTable() {
        std::mt19937_64 rng(0x9E3779B97F4A7C15ULL);
        for (int x = 0; x < 19; ++x)
            for (int y = 0; y < 19; ++y)
                for (int k = 0; k < 3; ++k)
                    table[x][y][k] = rng();
        side = rng();
    }
};
const ZobristTable& zobrist() {
    static ZobristTable z;
    return z;
}

Game::Game()
    : m_chess{{0}}
    , m_step{0}
    , m_win{TIE}
    , m_type{1}
    , m_timeLimitMs{searchConfig.timeLimitMs}
    , m_timedOut{false}
{
    m_search.reserve(361);
    m_tt.resize(TT_SIZE);
    //qDebug() << this->parent();
}
void Game::setType(int newType)
{
    m_type = newType;
}

void Game::stepadd()
{
    m_step++;
}

void Game::stepdec()
{
    m_step--;
}
int Game::type() const
{
    return m_type;
}

int Game::step() const
{
    return m_step;
}


void Game::whoFirst(int type)
{
    if(type==1)
        ourFirstPlay();

}

void Game::whoPlay(int type)
{
    if(m_type==type)
        ourplay();
}

void Game::gameundo(int x, int y)
{
    clearStone(x, y);
    rebuildSearch();
}

void Game::whoWin()
{
    QMessageBox msgBox;
    switch (m_win) {
    case TIE:
        msgBox.setText("双方平局");
        break;
    case WIN_BLACK:
        msgBox.setText("黑方胜利");
        break;
    case WIN_WHITE:
        msgBox.setText("白方胜利");
        break;
    }
    msgBox.setInformativeText("是否保存棋谱文件");
    msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Save);
    int ret = msgBox.exec();
    switch (ret) {
    case QMessageBox::Save:
        emit save_signal();
        break;
    case QMessageBox::Discard:
        // Don't Save was clicked
        break;
    case QMessageBox::Cancel:
        // Cancel was clicked
        break;
    default:
        // should never be reached
        break;
    }
}
void Game::ourFirstPlay()
{
    vector<QPoint> temp;
    QPoint location{ 9,9 };
    set(location);//落子
    add(location, temp);//扩展搜索队列
    emit chessChange(location,m_type);
    emit undoenable();//发送撤回键可以用的信号

    swap();
}

void Game::ourplay()
{
    emit restart(m_type);
    bestPlay();
}

bool Game::isEnd()
{
    for (int i = 0; i < 14; i++)
        for (int j = 0; j < 14; j++) {
            int end1 = 0, end2 = 0, end3 = 0, end4 = 0;
            for (int k = 0; k < 6; k++) {
                if (m_chess[i][j + k] == 2)
                    end1 += -1;
                else
                    end1 += m_chess[i][j + k]; //横着
                if (m_chess[i + k][j] == 2)
                    end2 += -1;
                else
                    end2 += m_chess[i + k][j]; //竖着
                if (m_chess[i + k][j + k] == 2)
                    end3 += -1;
                else
                    end3 += m_chess[i + k][j + k]; //斜着
                if (m_chess[i + k][j + 5 - k] == 2)
                    end4 += -1;
                else
                    end4 += m_chess[i + k][j + 5 - k]; //斜着
            }
            if (end1 / 6 < 0 || end2 / 6 < 0 || end3 / 6 < 0 || end4 / 6 < 0) {
                m_win = WIN_WHITE;
                return true;
            }
            if (end1 / 6 > 0 || end2 / 6 > 0 || end3 / 6 > 0 || end4 / 6 > 0) {
                m_win = WIN_BLACK;
                return true;
            }
        }

    for (int i = 0; i < 14; i++)
        for (int j = 14; j < 19; j++) {
            int end2 = 0;
            for (int k = 0; k < 6; k++) {
                if (m_chess[i + k][j] == 2)
                    end2 += -1;
                else
                    end2 += m_chess[i + k][j]; //竖着
            }
            if (end2 / 6 < 0) {
                m_win = WIN_WHITE;
                return true;
            }
            if (end2 / 6 > 0) {
                m_win = WIN_BLACK;
                return true;
            }
        }
    for (int i = 14; i < 19; i++)
        for (int j = 0; j < 14; j++) {
            int end1 = 0;
            for (int k = 0; k < 6; k++) {
                if (m_chess[i][j + k] == 2)
                    end1 += -1;
                else
                    end1 += m_chess[i][j + k]; //横着
            }
            if (end1 / 6 < 0) {
                m_win = WIN_WHITE;
                return true;
            }
            if (end1 / 6 > 0) {
                m_win = WIN_BLACK;
                return true;
            }
        }
    int countNull = 0;
    for (int i = 0; i < 19; i++)
        for (int j = 0; j < 19; j++) {
            if (m_chess[i][j] == 0)
                countNull++;
        }
    if (countNull == 0) {
        m_win = TIE;
        return true;
    }
    return false;
}

void Game::swap()
{
    m_type==1?m_type=2:m_type=1;
    m_step==0?m_step=2:m_step=0;
    emit stop();
    emit restart(m_type);
}

void Game::set(QPoint location)
{
    int x = location.x();
    int y = location.y();

    placeStone(x, y, m_type);
    if (contain(m_search, location))           //要先删掉再扩展
    {
        for (vector<QPoint>::iterator it = m_search.begin(); it != m_search.end(); it++)
            if (*it == location)
            {
                m_search.erase(it);
                break;
            }
    }
    m_step++;
}

void Game::add(QPoint location, std::vector<QPoint> &lastAdd)
{
    int x = location.x();
    int y = location.y();
    for (int i = -2; i < 3; i++)
    {
        if (x + i >= 0 && x + i <= 18)
        {
            QPoint temp = { x + i,y };
            if (m_chess[x + i][y] == 0 && !contain(m_search, temp))
            {
                m_search.push_back(temp);
                lastAdd.push_back(temp);
            }
        }
        if (y + i >= 0 && y + i <= 18)
        {
            QPoint temp = { x ,y + i };
            if (m_chess[x][y + i] == 0 && !contain(m_search, temp))
            {
                m_search.push_back(temp);
                lastAdd.push_back(temp);
            }
        }
        if (x + i >= 0 && x + i <= 18 && y + i >= 0 && y + i <= 18)
        {
            QPoint temp1 = { x + i ,y + i };
            if (m_chess[x + i][y + i] == 0 && !contain(m_search, temp1))
            {
                m_search.push_back(temp1);
                lastAdd.push_back(temp1);
            }
        }
        if (x - i >= 0 && x - i <= 18 && y + i >= 0 && y + i <= 18)
        {
            QPoint temp2 = { x - i ,y + i };
            if (m_chess[x - i][y + i] == 0 && !contain(m_search, temp2))
            {
                m_search.push_back(temp2);
                lastAdd.push_back(temp2);
            }
        }
    }
}

void Game::retract(QPoint location,vector<QPoint>&lastAdd)
{
    int x = location.x();
    int y = location.y();
    clearStone(x, y);
    m_search.push_back(location);
    if (!lastAdd.empty())
    {
        for (const QPoint& p : lastAdd)
            m_search.erase(std::remove(m_search.begin(), m_search.end(), p), m_search.end());
    }
    lastAdd.clear();
    if (m_step == 0)
    {
        m_step = 2;
        m_type == 1 ? m_type = 2 : m_type = 1;
    }
    m_step--;
}

void Game::make(QPoint a, QPoint b)
{
    m_last = a;
    m_llast = b;
    placeStone(a.x(), a.y(), m_type);
    placeStone(b.x(), b.y(), m_type);
}

void Game::unmake(QPoint a, QPoint b)
{
    clearStone(a.x(), a.y());
    clearStone(b.x(), b.y());
}

void Game::analyzeDir(int px, int py, int dx, int dy,
                      int skipX, int skipY, int dirIdx)
{
    for (int k = 0; k <= 5; k++) {
        int sx = px - k * dx;
        int sy = py - k * dy;
        bool valid = true;
        for (int t = 0; t < 6; t++) {
            int cx = sx + t * dx;
            int cy = sy + t * dy;
            if (cx < 0 || cx > 18 || cy < 0 || cy > 18) {
                valid = false;
                break;
            }
        }
        if (!valid) continue;
        if (skipX >= 0) {
            bool shared = false;
            for (int t = 0; t < 6; t++) {
                if (sx + t * dx == skipX && sy + t * dy == skipY) {
                    shared = true;
                    break;
                }
            }
            if (shared) continue;
        }
        int number = 0;
        for (int t = 0; t < 6; t++) {
            int cx = sx + t * dx;
            int cy = sy + t * dy;
            int v = m_chess[cx][cy];
            if (v == 2) number += 7;
            else number += v;
        }
        if (number == 0 || (number > 6 && number % 7 != 0))
            continue;
        if (number < 7) {
            if (m_type == 1) {
                numberOfMyRoad[number]++;
                if (number == 4) m_fourConnect[dirIdx][0]++;
            } else {
                numberOfEnemyRoad[number]++;
            }
        } else {
            if (m_type == 1) {
                numberOfEnemyRoad[number / 7]++;
            } else {
                numberOfMyRoad[number / 7]++;
                if (number / 7 == 4) m_fourConnect[dirIdx][0]++;
            }
        }
    }
}
void Game::AnalyFourConnect()
{
    int x1 = m_last.x(), y1 = m_last.y();
    int x2 = m_llast.x(), y2 = m_llast.y();

    //判断是否为活四
    if (numberOfMyRoad[4] > 0) {
        if (m_fourConnect[0][0] > 0) {
            for (int i = y1 - 5 > 0 ? y1 - 5 : 0; i <= y1 && i + 7 < 19; i++) {
                if (m_chess[x1][i] == m_chess[x1][i + 1] && m_chess[x1][i + 1] == m_chess[x1][i + 6]
                    && m_chess[x1][i + 6] == m_chess[x1][i + 7] && m_chess[x1][i + 7] == 0
                    && m_chess[x1][i + 2] == m_chess[x1][i + 3] && m_chess[x1][i + 3] == m_chess[x1][i + 4]
                    && m_chess[x1][i + 4] == m_chess[x1][i + 5]
                    && (m_chess[x1][i + 5] == 1 || m_chess[x1][i + 5] == 2)) {
                    m_fourConnect[0][1] += 1;
                }
            }
            for (int i = y2 - 5 > 0 ? y2 - 5 : 0; i <= y2 && i + 7 < 19; i++) {
                if (m_chess[x2][i] == m_chess[x2][i + 1] && m_chess[x2][i + 1] == m_chess[x2][i + 6]
                    && m_chess[x2][i + 6] == m_chess[x2][i + 7] && m_chess[x2][i + 7] == 0
                    && m_chess[x2][i + 2] == m_chess[x2][i + 3] && m_chess[x2][i + 3] == m_chess[x2][i + 4]
                    && m_chess[x2][i + 4] == m_chess[x2][i + 5]
                    && (m_chess[x2][i + 5] == 1 || m_chess[x2][i + 5] == 2)) {
                    m_fourConnect[0][1] += 1;
                }
            }
        }

        if (m_fourConnect[1][0] > 0) {
            for (int i = x1 - 5 > 0 ? x1 - 5 : 0; i <= x1 && i + 7 < 19; i++) {
                if (m_chess[i][y1] == m_chess[i + 1][y1] && m_chess[i + 1][y1] == m_chess[i + 6][y1]
                    && m_chess[i + 6][y1] == m_chess[i + 7][y1] && m_chess[i + 7][y1] == 0
                    && m_chess[i + 2][y1] == m_chess[i + 3][y1] && m_chess[i + 3][y1] == m_chess[i + 4][y1]
                    && m_chess[i + 4][y1] == m_chess[i + 5][y1]
                    && (m_chess[i + 5][y1] == 1 || m_chess[i + 5][y1] == 2)) {
                    m_fourConnect[1][1] += 1;
                }
            }
            for (int i = x2 - 5 > 0 ? x2 - 5 : 0; i <= x2 && i + 7 < 19; i++) {
                if (m_chess[i][y2] == m_chess[i + 1][y2] && m_chess[i + 1][y2] == m_chess[i + 6][y2]
                    && m_chess[i + 6][y2] == m_chess[i + 7][y2] && m_chess[i + 7][y2] == 0
                    && m_chess[i + 2][y2] == m_chess[i + 3][y2] && m_chess[i + 3][y2] == m_chess[i + 4][y2]
                    && m_chess[i + 4][y2] == m_chess[i + 5][y2]
                    && (m_chess[i + 5][y2] == 1 || m_chess[i + 5][y2] == 2)) {
                    m_fourConnect[1][1] += 1;
                }
            }
        }

        if (m_fourConnect[2][0] > 0) {
            int i, j;
            if (x1 - 5 < 0 || y1 - 5 < 0) {
                int t = std::min(x1, y1);
                i = x1 - t;
                j = y1 - t;
            } else {
                i = x1 - 5;
                j = y1 - 5;
            }
            for (; i <= x1 && i + 7 < 19 && j <= y1 && j + 7 < 19; i++, j++) {
                if (m_chess[i][j] == m_chess[i + 1][j + 1] && m_chess[i + 1][j + 1] == m_chess[i + 6][j + 6]
                    && m_chess[i + 6][j + 6] == m_chess[i + 7][j + 7] && m_chess[i + 7][j + 7] == 0
                    && m_chess[i + 2][j + 2] == m_chess[i + 3][j + 3] && m_chess[i + 3][j + 3] == m_chess[i + 4][j + 4]
                    && m_chess[i + 4][j + 4] == m_chess[i + 5][j + 5]
                    && (m_chess[i + 5][j + 5] == 1 || m_chess[i + 5][j + 5] == 2)) {
                    m_fourConnect[2][1] += 1;
                }
            }
            if (x2 - 5 < 0 || y2 - 5 < 0)
            {
                int t = std::min(x2, y2);
                i = x2 - t;
                j = y2 - t;
            }
            else
            {
                i = x2 - 5;
                j = y2 - 5;
            }
            for (; i <= x2 && i + 7 < 19 && j <= y2 && j + 7 < 19; i++, j++) {
                if (m_chess[i][j] == m_chess[i + 1][j + 1] && m_chess[i + 1][j + 1] == m_chess[i + 6][j + 6]
                    && m_chess[i + 6][j + 6] == m_chess[i + 7][j + 7] && m_chess[i + 7][j + 7] == 0
                    && m_chess[i + 2][j + 2] == m_chess[i + 3][j + 3] && m_chess[i + 3][j + 3] == m_chess[i + 4][j + 4]
                    && m_chess[i + 4][j + 4] == m_chess[i + 5][j + 5]
                    && (m_chess[i + 5][j + 5] == 1 || m_chess[i + 5][j + 5] == 2)) {
                    m_fourConnect[2][1] += 1;
                }
            }
        }

        if (m_fourConnect[3][0] > 0) {
            int i, j;
            if (x1 - 5 < 0 || y1 + 5 > 18)
            {
                int t = std::min(x1, 18 - y1);
                i = x1 - t;
                j = y1 + t;
            }
            else
            {
                i = x1 - 5;
                j = y1 + 5;
            }
            for (; i <= x1 && i + 7 < 19 && j >= y1 && j - 7 >= 0; i++, j--) {
                if (m_chess[i][j] == m_chess[i + 1][j - 1] && m_chess[i + 1][j - 1] == m_chess[i + 6][j - 6]
                    && m_chess[i + 6][j - 6] == m_chess[i + 7][j - 7] && m_chess[i + 7][j - 7] == 0
                    && m_chess[i + 2][j - 2] == m_chess[i + 3][j - 3] && m_chess[i + 3][j - 3] == m_chess[i + 4][j - 4]
                    && m_chess[i + 4][j - 4] == m_chess[i + 5][j - 5]
                    && (m_chess[i + 5][j - 5] == 1 || m_chess[i + 5][j - 5] == 2)) {
                    m_fourConnect[3][1] += 1;
                }
            }
            if (x2 - 5 < 0 || y2 + 5 > 18)
            {
                int t = std::min(x2, 18 - y2);
                i = x2 - t;
                j = y2 + t;
            }
            else
            {
                i = x2 - 5;
                j = y2 + 5;
            }
            for (; i <= x2 && i + 7 < 19 && j >= y2 && j - 7 >= 0; i++, j--) {
                if (m_chess[i][j] == m_chess[i + 1][j - 1] && m_chess[i + 1][j - 1] == m_chess[i + 6][j - 6]
                    && m_chess[i + 6][j - 6] == m_chess[i + 7][j - 7] && m_chess[i + 7][j - 7] == 0
                    && m_chess[i + 2][j - 2] == m_chess[i + 3][j - 3] && m_chess[i + 3][j - 3] == m_chess[i + 4][j - 4]
                    && m_chess[i + 4][j - 4] == m_chess[i + 5][j - 5]
                    && (m_chess[i + 5][j - 5] == 1 || m_chess[i + 5][j - 5] == 2)) {
                    m_fourConnect[3][1] += 1;
                }
            }
        }
    }
}

bool Game::CheckWinningPatterns()
{
    int liveCount = 0;
    for (int d = 0; d < 4; d++)
        if (m_fourConnect[d][1] > 0) liveCount++;
    if (liveCount >= 2) return true;
    if (liveCount == 1) {
        for (int d = 0; d < 4; d++) {
            if (m_fourConnect[d][1] == 0) continue;
            for (int e = 0; e < 4; e++)
                if (e != d && m_fourConnect[e][0] > 0) return true;
        }
    }
    return false;
}
void Game::resetFourConnect()
{
    for (int d = 0; d < 4; d++)
        for (int i = 0; i < 2; i++)
            m_fourConnect[d][i] = 0;
}

int Game::estimate()
{
    int pre = 0, after = 0;
    int x1 = m_last.x(), y1 = m_last.y();
    int x2 = m_llast.x(), y2 = m_llast.y();
    int type = m_chess[x1][y1];
    const int DIR[4][2] = {{0,1}, {1,0}, {1,1}, {1,-1}};

    clearStone(x1, y1);
    clearStone(x2, y2);
    for (int i = 0; i < 7; i++) {
        numberOfMyRoad[i] = 0;
        numberOfEnemyRoad[i] = 0;
    }
    resetFourConnect();
    for (int d = 0; d < 4; d++) {
        analyzeDir(x1, y1, DIR[d][0], DIR[d][1], -1, -1, d);
        analyzeDir(x2, y2, DIR[d][0], DIR[d][1], x1, y1, d);
    }
    for (int i = 1; i <= 6; i++)
        pre += numberOfMyRoad[i] * scoreOfMyRoad[i] - numberOfEnemyRoad[i] * scoreOfEnemyRoad[i];

    resetFourConnect();
    placeStone(x1, y1, type);
    placeStone(x2, y2, type);
    for (int i = 0; i < 7; i++) {
        numberOfMyRoad[i] = 0;
        numberOfEnemyRoad[i] = 0;
    }
    for (int d = 0; d < 4; d++) {
        analyzeDir(x1, y1, DIR[d][0], DIR[d][1], -1, -1, d);
        analyzeDir(x2, y2, DIR[d][0], DIR[d][1], x1, y1, d);
    }
    AnalyFourConnect();
    for (int i = 1; i <= 6; i++)
        after += numberOfMyRoad[i] * scoreOfMyRoad[i] - numberOfEnemyRoad[i] * scoreOfEnemyRoad[i];
    if (CheckWinningPatterns())
        after += scoreOfWinningChessPattern[1];
    resetFourConnect();
    return after - pre;
}

bool Game::contain(const std::vector<QPoint>& points, QPoint value) const
{

    for (auto it = points.begin(); it != points.end(); ++it)
    {
        if ((*it) == value)
            return true;
    }
    return false;
}

quint64 Game::boardHash() const
{
    const ZobristTable& z = zobrist();
    quint64 h = 0;
    for (int x = 0; x < 19; ++x)
        for (int y = 0; y < 19; ++y)
            if (m_chess[x][y])
                h ^= z.table[x][y][m_chess[x][y]];
    if (m_type == 1) h ^= z.side;
    return h;
}

static int lineShape(int run, int open)
{
    const auto &weights = Game::searchConfig;
    if (run >= 6) return weights.shapeSix;
    if (run == 5) return open >= 1 ? weights.shapeFive : 0;
    if (run == 4) return open == 2 ? weights.shapeOpenFour : (open == 1 ? weights.shapeClosedFour : 0);
    if (run == 3) return open == 2 ? weights.shapeOpenThree : (open == 1 ? weights.shapeClosedThree : 0);
    if (run == 2) return open == 2 ? weights.shapeOpenTwo : 0;
    return weights.shapeSingle;
}

namespace {
constexpr int kDirections[4][2] = {{0, 1}, {1, 0}, {1, 1}, {1, -1}};

inline bool onBoard(int x, int y)
{
    return x >= 0 && x < 19 && y >= 0 && y < 19;
}

inline void lineStart(int x, int y, int d, int& sx, int& sy)
{
    if (d == 0) { sx = x; sy = 0; }
    else if (d == 1) { sx = 0; sy = y; }
    else if (d == 2) { const int k = std::min(x, y); sx = x - k; sy = y - k; }
    else { const int k = std::min(x, 18 - y); sx = x - k; sy = y + k; }
}

inline int lineId(int d, int x, int y)
{
    if (d == 0) return x;
    if (d == 1) return 19 + y;
    if (d == 2) return 38 + (x - y + 18);
    return 75 + (x + y);
}

// The 112 maximal lines cover every run exactly once. lineShape() is
// unchanged; this only changes how the same sum is maintained.
void lineScorePair(const int (&board)[19][19], int sx, int sy, int dx, int dy,
                   int& black, int& white)
{
    black = 0;
    white = 0;
    int x = sx, y = sy;
    while (onBoard(x, y)) {
        const int c = board[x][y];
        if (c == 0) { x += dx; y += dy; continue; }
        int run = 1;
        int nx = x + dx, ny = y + dy;
        while (onBoard(nx, ny) && board[nx][ny] == c) {
            ++run;
            nx += dx;
            ny += dy;
        }
        const int px = x - dx, py = y - dy;
        const int open = (onBoard(px, py) && board[px][py] == 0 ? 1 : 0)
                       + (onBoard(nx, ny) && board[nx][ny] == 0 ? 1 : 0);
        const int shape = lineShape(run, open);
        if (c == 1) black += shape;
        else white += shape;
        x = nx;
        y = ny;
    }
}

struct ColorDelta {
    int black = 0;
    int white = 0;
};

struct AdjacentRun {
    int length = 0;
    int color = 0;
    int open = 0;
};

AdjacentRun adjacentRun(const int (&board)[19][19], int x, int y, int dx, int dy)
{
    AdjacentRun run;
    int nx = x + dx, ny = y + dy;
    if (!onBoard(nx, ny)) return run;
    const int color = board[nx][ny];
    if (color == 0) return run;
    run.color = color;
    while (run.length < 6 && onBoard(nx, ny) && board[nx][ny] == color) {
        ++run.length;
        nx += dx;
        ny += dy;
    }
    run.open = (onBoard(nx, ny) && board[nx][ny] == 0) ? 1 : 0;
    return run;
}

// Exact score change for placing `color` at the empty point (x, y), split by
// color so the per-line cache can be updated in O(1) instead of rescanned.
ColorDelta lineDeltaForPlacing(const int (&board)[19][19], int x, int y, int dx, int dy, int color)
{
    const AdjacentRun left = adjacentRun(board, x, y, -dx, -dy);
    const AdjacentRun right = adjacentRun(board, x, y, dx, dy);
    const bool leftOpen = onBoard(x - dx, y - dy) && board[x - dx][y - dy] == 0;
    const bool rightOpen = onBoard(x + dx, y + dy) && board[x + dx][y + dy] == 0;
    ColorDelta before, after;
    if (left.length > 0) {
        const int shape = lineShape(left.length, left.open + 1);
        (left.color == 1 ? before.black : before.white) += shape;
    }
    if (right.length > 0) {
        const int shape = lineShape(right.length, right.open + 1);
        (right.color == 1 ? before.black : before.white) += shape;
    }
    if (left.length > 0 && right.length > 0 && left.color == color && right.color == color) {
        (color == 1 ? after.black : after.white) +=
            lineShape(std::min(6, left.length + right.length + 1), left.open + right.open);
    } else if (left.length > 0 && left.color == color) {
        (color == 1 ? after.black : after.white) +=
            lineShape(std::min(6, left.length + 1), left.open + (rightOpen ? 1 : 0));
    } else if (right.length > 0 && right.color == color) {
        (color == 1 ? after.black : after.white) +=
            lineShape(std::min(6, right.length + 1), right.open + (leftOpen ? 1 : 0));
    } else {
        (color == 1 ? after.black : after.white) +=
            lineShape(1, (leftOpen ? 1 : 0) + (rightOpen ? 1 : 0));
    }
    if (left.length > 0 && left.color != color)
        (left.color == 1 ? after.black : after.white) += lineShape(left.length, left.open);
    if (right.length > 0 && right.color != color)
        (right.color == 1 ? after.black : after.white) += lineShape(right.length, right.open);
    return {after.black - before.black, after.white - before.white};
}

// Exact but slow fallback for callers that pass a piece outside {1, 2}; the
// production search only uses 1/2, while tests may probe other values.
int boardScoreForPiece(const int (&board)[19][19], int piece)
{
    const int DIR[4][2] = {{0, 1}, {1, 0}, {1, 1}, {1, -1}};
    int score = 0;
    for (int x = 0; x < 19; ++x) {
        for (int y = 0; y < 19; ++y) {
            const int c = board[x][y];
            if (c == 0) continue;
            for (int d = 0; d < 4; ++d) {
                const int dx = DIR[d][0], dy = DIR[d][1];
                const int px = x - dx, py = y - dy;
                if (onBoard(px, py) && board[px][py] == c) continue;
                int run = 0, cx = x, cy = y;
                while (onBoard(cx, cy) && board[cx][cy] == c) {
                    ++run;
                    cx += dx;
                    cy += dy;
                }
                const int open = (onBoard(px, py) && board[px][py] == 0 ? 1 : 0)
                               + (onBoard(cx, cy) && board[cx][cy] == 0 ? 1 : 0);
                const int shape = lineShape(run, open);
                score += (c == piece ? shape : -shape);
            }
        }
    }
    return score;
}
}

int Game::evaluateBoard()
{
    int black = 0, white = 0;
    for (int i = 0; i < 112; ++i) {
        black += m_lineScore[0][i];
        white += m_lineScore[1][i];
    }
    if (m_type == 1) return black - white;
    if (m_type == 2) return white - black;
    return -(black + white);
}

int Game::evaluatePoint(int x, int y, int piece)
{
    return evaluatePair(QPoint(x, y), QPoint(x, y), piece);
}

int Game::evaluatePair(QPoint a, QPoint b, int piece)
{
    const auto scoreOf = [piece](const ColorDelta& delta) {
        return piece == 1 ? delta.black - delta.white : delta.white - delta.black;
    };
    if (piece != 1 && piece != 2) {
        const int ax = a.x(), ay = a.y(), bx = b.x(), by = b.y();
        const bool emptyA = onBoard(ax, ay) && m_chess[ax][ay] == 0;
        const bool emptyB = onBoard(bx, by) && m_chess[bx][by] == 0;
        const int before = boardScoreForPiece(m_chess, piece);
        if (emptyA) m_chess[ax][ay] = piece;
        if (emptyB && !(ax == bx && ay == by)) m_chess[bx][by] = piece;
        const int after = boardScoreForPiece(m_chess, piece);
        if (emptyA) m_chess[ax][ay] = 0;
        if (emptyB && !(ax == bx && ay == by)) m_chess[bx][by] = 0;
        return after - before;
    }
    const bool emptyA = onBoard(a.x(), a.y()) && m_chess[a.x()][a.y()] == 0;
    const bool emptyB = onBoard(b.x(), b.y()) && m_chess[b.x()][b.y()] == 0;
    if (!emptyA && !emptyB) return 0;
    if (a == b) {
        int total = 0;
        for (int d = 0; d < 4; ++d) {
            int sx, sy;
            lineStart(a.x(), a.y(), d, sx, sy);
            total += scoreOf(lineDeltaForPlacing(m_chess, a.x(), a.y(),
                                                 kDirections[d][0], kDirections[d][1], piece));
        }
        return total;
    }
    struct LineRef {
        int d;
        int sx;
        int sy;
        int mask;
    };
    LineRef lines[8];
    int lineCount = 0;
    const auto addPoint = [&](QPoint p, int bit) {
        for (int d = 0; d < 4; ++d) {
            int sx, sy;
            lineStart(p.x(), p.y(), d, sx, sy);
            int i = 0;
            for (; i < lineCount; ++i)
                if (lines[i].d == d && lines[i].sx == sx && lines[i].sy == sy) {
                    lines[i].mask |= bit;
                    break;
                }
            if (i == lineCount) {
                lines[lineCount].d = d;
                lines[lineCount].sx = sx;
                lines[lineCount].sy = sy;
                lines[lineCount].mask = bit;
                ++lineCount;
            }
        }
    };
    if (emptyA) addPoint(a, 1);
    if (emptyB) addPoint(b, 2);
    int total = 0;
    for (int i = 0; i < lineCount; ++i) {
        const int dx = kDirections[lines[i].d][0];
        const int dy = kDirections[lines[i].d][1];
        if (lines[i].mask == 3) {
            int black = 0, white = 0;
            lineScorePair(m_chess, lines[i].sx, lines[i].sy, dx, dy, black, white);
            const int before = piece == 1 ? black - white : white - black;
            m_chess[a.x()][a.y()] = piece;
            m_chess[b.x()][b.y()] = piece;
            lineScorePair(m_chess, lines[i].sx, lines[i].sy, dx, dy, black, white);
            const int after = piece == 1 ? black - white : white - black;
            m_chess[a.x()][a.y()] = 0;
            m_chess[b.x()][b.y()] = 0;
            total += after - before;
        } else {
            const QPoint p = lines[i].mask == 1 ? a : b;
            total += scoreOf(lineDeltaForPlacing(m_chess, p.x(), p.y(), dx, dy, piece));
        }
    }
    return total;
}

void Game::placeStone(int x, int y, int color)
{
    if (m_chess[x][y] != 0) clearStone(x, y);
    m_chess[x][y] = 0;
    for (int d = 0; d < 4; ++d) {
        int sx, sy;
        lineStart(x, y, d, sx, sy);
        const ColorDelta delta = lineDeltaForPlacing(m_chess, x, y,
                                                     kDirections[d][0], kDirections[d][1], color);
        const int id = lineId(d, sx, sy);
        m_lineScore[0][id] += delta.black;
        m_lineScore[1][id] += delta.white;
    }
    m_chess[x][y] = color;
    if (searchConfig.incrementalTactics && color != 0) updateTacticalWindows(x, y, color, 1);
}

void Game::clearStone(int x, int y)
{
    const int color = m_chess[x][y];
    if (color == 0) return;
    if (searchConfig.incrementalTactics) updateTacticalWindows(x, y, color, -1);
    m_chess[x][y] = 0;
    for (int d = 0; d < 4; ++d) {
        int sx, sy;
        lineStart(x, y, d, sx, sy);
        const ColorDelta delta = lineDeltaForPlacing(m_chess, x, y,
                                                     kDirections[d][0], kDirections[d][1], color);
        const int id = lineId(d, sx, sy);
        m_lineScore[0][id] -= delta.black;
        m_lineScore[1][id] -= delta.white;
    }
}

void Game::recomputeLineScores()
{
    for (int c = 0; c < 2; ++c)
        for (int i = 0; i < 112; ++i)
            m_lineScore[c][i] = 0;
    const auto add = [&](int d, int sx, int sy) {
        int black = 0, white = 0;
        lineScorePair(m_chess, sx, sy, kDirections[d][0], kDirections[d][1], black, white);
        const int id = lineId(d, sx, sy);
        m_lineScore[0][id] = black;
        m_lineScore[1][id] = white;
    };
    for (int x = 0; x < 19; ++x) add(0, x, 0);
    for (int y = 0; y < 19; ++y) add(1, 0, y);
    for (int y = 0; y < 19; ++y) add(2, 0, y);
    for (int x = 1; x < 19; ++x) add(2, x, 0);
    for (int y = 0; y < 19; ++y) add(3, 0, y);
    for (int x = 1; x < 19; ++x) add(3, x, 18);
}


bool Game::hasSix(int x, int y, int piece)
{
    const int DIR[4][2] = {{0,1}, {1,0}, {1,1}, {1,-1}};
    for (int d = 0; d < 4; d++) {
        int dx = DIR[d][0], dy = DIR[d][1];
        int cnt = 1;
        for (int s = 1; s < 6; s++) {
            int nx = x + dx * s, ny = y + dy * s;
            if (nx < 0 || nx > 18 || ny < 0 || ny > 18 || m_chess[nx][ny] != piece) break;
            cnt++;
        }
        for (int s = 1; s < 6; s++) {
            int nx = x - dx * s, ny = y - dy * s;
            if (nx < 0 || nx > 18 || ny < 0 || ny > 18 || m_chess[nx][ny] != piece) break;
            cnt++;
        }
        if (cnt >= 6) return true;
    }
    return false;
}

bool Game::createsThreat(QPoint a, QPoint b, int piece) const
{
    const QPoint points[2] = {a, b};
    for (const QPoint& p : points) {
        for (int d = 0; d < 4; ++d) {
            const int dx = kDirections[d][0], dy = kDirections[d][1];
            for (int offset = -5; offset <= 0; ++offset) {
                const int sx = p.x() + offset * dx;
                const int sy = p.y() + offset * dy;
                int own = 0, enemy = 0, holes = 0;
                bool valid = true;
                for (int k = 0; k < 6; ++k) {
                    const int x = sx + k * dx, y = sy + k * dy;
                    if (!onBoard(x, y)) { valid = false; break; }
                    const int c = m_chess[x][y];
                    if (c == 0) ++holes;
                    else if (c == piece) ++own;
                    else ++enemy;
                }
                if (valid && enemy == 0 && own >= 4 && holes <= 2) return true;
            }
        }
    }
    return false;
}

void Game::updateTacticalWindows(int x, int y, int color, int delta)
{
    for (int index : sixWindows().incidence[x * 19 + y]) {
        auto& counts = m_windowCounts[index];
        const bool before = counts[2] >= 4 && (counts[0] == 0 || counts[1] == 0);
        counts[2] += delta;
        if (color == 1 || color == 2) counts[color - 1] += delta;
        const bool after = counts[2] >= 4 && (counts[0] == 0 || counts[1] == 0);
        m_forcingWindowCount += int(after) - int(before);
    }
}

void Game::rebuildTacticalWindows()
{
    for (auto& window : m_windowCounts) for (auto& count : window) count = 0;
    m_forcingWindowCount = 0;
    for (int x = 0; x < 19; ++x) for (int y = 0; y < 19; ++y)
        if (m_chess[x][y] != 0) updateTacticalWindows(x, y, m_chess[x][y], 1);
}

void Game::rebuildSearch(bool rebuildCaches)
{
    m_search.reserve(361);
    m_search.clear();
    for (int x = 0; x < 19; x++)
        for (int y = 0; y < 19; y++) {
            if (m_chess[x][y] != 0) continue;
            bool near = false;
            for (int dx = -2; dx <= 2 && !near; dx++)
                for (int dy = -2; dy <= 2 && !near; dy++)
                    if (x + dx >= 0 && x + dx < 19 && y + dy >= 0 && y + dy < 19
                        && m_chess[x + dx][y + dy] != 0)
                        near = true;
            if (near) m_search.push_back({x, y});
        }
    if (rebuildCaches || !searchConfig.incrementalTactics) {
        recomputeLineScores();
        if (searchConfig.incrementalTactics) rebuildTacticalWindows();
    }
}

int Game::tacticalMoves(int pieceType, std::vector<Move>& out, int stonesRemaining,
                        bool enumerate, qint64 deadlineMs)
{
    out.clear();
    const auto interrupted = [&]() {
        return timedOut() || (deadlineMs >= 0 && m_timer.elapsed() >= deadlineMs);
    };
    if (interrupted()) return -2;
    const bool cached = searchConfig.incrementalTactics && (pieceType == 1 || pieceType == 2);
    if (cached && m_forcingWindowCount == 0) return 0;
    out.reserve(361);
    int empty[361], emptyCount = 0;
    for (int x = 0; x < 19; ++x)
        for (int y = 0; y < 19; ++y)
            if (m_chess[x][y] == 0) empty[emptyCount++] = x * 19 + y;
    if (interrupted()) return -2;
    if (emptyCount < stonesRemaining) return 0;

    // At most 924 six-cell windows. A singleton uses the same index twice.
    int threats[924][2], threatCount = 0;
    if (cached) {
        const auto& geometry = sixWindows();
        // Geometry uses the legacy x/y/direction order, preserving move order.
        for (int index = 0; index < 924; ++index) {
            if ((index & 31) == 0 && interrupted()) { out.clear(); return -2; }
            const auto& counts = m_windowCounts[index];
            const int own = counts[pieceType - 1], enemy = counts[2] - own;
            const int holeCount = 6 - counts[2];
            if ((own && enemy) || holeCount == 0 || holeCount > 2) continue;
            int holes[2], n = 0;
            for (int point : geometry.windows[index])
                if (m_chess[point / 19][point % 19] == 0) holes[n++] = point;
            if (enemy == 0 && own >= 4 && holeCount <= stonesRemaining) {
                int a = holes[0], b = holes[holeCount - 1];
                if (a == b && stonesRemaining == 2)
                    for (int i = 0; i < emptyCount; ++i)
                        if (empty[i] != a) { b = empty[i]; break; }
                out.emplace_back(QPoint(a / 19, a % 19), QPoint(b / 19, b % 19), WIN_SCORE);
                if (interrupted()) { out.clear(); return -2; }
                return 1;
            }
            if (own == 0 && enemy >= 4) {
                threats[threatCount][0] = holes[0];
                threats[threatCount++][1] = holes[holeCount - 1];
            }
        }
    } else {
        const int DIR[4][2] = {{0,1}, {1,0}, {1,1}, {1,-1}};
        for (int x = 0; x < 19; ++x) {
            if (interrupted()) { out.clear(); return -2; }
            for (int y = 0; y < 19; ++y)
                for (const auto& dir : DIR) {
                    int ex = x + 5 * dir[0], ey = y + 5 * dir[1];
                    if (ex < 0 || ex >= 19 || ey < 0 || ey >= 19) continue;
                    int own = 0, enemy = 0;
                    int holes[2];
                    int holeCount = 0;
                    int cx = x, cy = y;
                    for (int t = 0; t < 6; ++t) {
                        int v = m_chess[cx][cy];
                        if (v == 0) {
                            if (holeCount == 2) { ++holeCount; break; }
                            holes[holeCount++] = cx * 19 + cy;
                        }
                        else if (v == pieceType) ++own;
                        else ++enemy;
                        if (own && enemy) break;
                        cx += dir[0]; cy += dir[1];
                    }
                    if ((own && enemy) || holeCount == 0 || holeCount > 2) continue;
                    if (enemy == 0 && own >= 4 && holeCount <= stonesRemaining) {
                        int a = holes[0], b = holes[holeCount - 1];
                        if (a == b && stonesRemaining == 2)
                            for (int i = 0; i < emptyCount; ++i)
                                if (empty[i] != a) { b = empty[i]; break; }
                        out.emplace_back(QPoint(a / 19, a % 19), QPoint(b / 19, b % 19), WIN_SCORE);
                        if (interrupted()) { out.clear(); return -2; }
                        return 1; // Winning now takes precedence over every defense.
                    }
                    if (own == 0 && enemy >= 4) {
                        threats[threatCount][0] = holes[0];
                        threats[threatCount++][1] = holes[holeCount - 1];
                    }
                }
        }
    }
    if (interrupted()) { out.clear(); return -2; }
    if (threatCount == 0) return 0;

    std::vector<unsigned char> seen(enumerate ? 361 * 361 : 0, 0);
    auto append = [&](int a, int b) {
        if (a == b) return;
        int i = a, j = b;
        if (i > j) std::swap(i, j);
        if (!seen[i * 361 + j]) {
            seen[i * 361 + j] = true;
            out.emplace_back(QPoint(a / 19, a % 19), QPoint(b / 19, b % 19), 0);
        }
    };
    // Every cover hits the first set. If a alone fails, b must hit the
    // first uncovered set; this enumerates all covers without quadratic search.
    for (int first = 0; first < 2; ++first) {
        if (first == 1 && threats[0][0] == threats[0][1]) continue;
        int a = threats[0][first];
        if (interrupted()) { out.clear(); return -2; }
        int uncovered = -1;
        for (int i = 0; i < threatCount; ++i)
            if (threats[i][0] != a && threats[i][1] != a) { uncovered = i; break; }
        if (uncovered == -1) {
            if (interrupted()) { out.clear(); return -2; }
            if (!enumerate) return 2;
            if (stonesRemaining == 1) out.emplace_back(QPoint(a / 19, a % 19), QPoint(a / 19, a % 19), 0);
            else for (int i = 0; i < emptyCount; ++i) append(a, empty[i]);
            continue;
        }
        if (stonesRemaining == 1) continue;
        for (int second = 0; second < 2; ++second) {
            if (second == 1 && threats[uncovered][0] == threats[uncovered][1]) continue;
            int b = threats[uncovered][second];
            bool coversAll = true;
            for (int i = 0; i < threatCount; ++i)
                if (threats[i][0] != a && threats[i][1] != a
                    && threats[i][0] != b && threats[i][1] != b) {
                    coversAll = false;
                    break;
                }
            if (coversAll) {
                if (interrupted()) { out.clear(); return -2; }
                if (!enumerate) return 2;
                append(a, b);
            }
        }
    }
    if (interrupted()) { out.clear(); return -2; }
    return out.empty() ? -1 : 2;
}

int Game::generateCandidates(int pieceType, std::vector<Move>& out,
                             int maxPoints, int maxPairs, int stonesRemaining, qint64 deadlineMs)
{
    const auto interrupted = [&]() {
        return timedOut() || (deadlineMs >= 0 && m_timer.elapsed() >= deadlineMs);
    };
    int tactic = tacticalMoves(pieceType, out, stonesRemaining, true, deadlineMs);
    if (tactic == -2 || interrupted()) { out.clear(); return -2; }
    if (tactic == 1) return tactic;
    if (tactic == 2) {
        for (Move& mv : out) {
            if (interrupted()) { out.clear(); return -2; }
            mv.setScore(evaluatePair(mv.step1(), mv.step2(), pieceType));
        }
        if (interrupted()) { out.clear(); return -2; }
        std::sort(out.begin(), out.end(),
                  [](const Move& a, const Move& b) { return a.score() > b.score(); });
        if (interrupted()) { out.clear(); return -2; }
        return tactic; // Never top-K/top-N prune required defenses or their fillers.
    }

    std::vector<QPoint> legal;
    legal.reserve(361);
    for (int x = 0; x < 19; ++x)
        for (int y = 0; y < 19; ++y)
            if (m_chess[x][y] == 0) legal.emplace_back(x, y);
    if (interrupted()) { out.clear(); return -2; }
    if (static_cast<int>(legal.size()) < stonesRemaining) return tactic;
    if (tactic == -1) {
        // The result is already proven lost; still supply a legal turn.
        out.emplace_back(legal[0], legal[stonesRemaining - 1], -WIN_SCORE);
        if (interrupted()) { out.clear(); return -2; }
        return tactic;
    }

    // 1. 单点差分评估并排序
    std::vector<std::pair<QPoint, int>> pts;
    const auto& search = m_search.empty() ? legal : m_search;
    pts.reserve(search.size());
    for (const QPoint& p : search) {
        if (interrupted()) { out.clear(); return -2; }
        if (m_chess[p.x()][p.y()] != 0) continue;
        int s = evaluatePoint(p.x(), p.y(), pieceType);
        pts.emplace_back(p, s);
    }
    if (interrupted()) { out.clear(); return -2; }
    std::sort(pts.begin(), pts.end(),
              [](const std::pair<QPoint, int>& a, const std::pair<QPoint, int>& b) {
                  return a.second > b.second;
              });
    if (interrupted()) { out.clear(); return -2; }
    int K = std::min((int)pts.size(), maxPoints);

    // Rank the complete turn delta, including synergy in shared windows.
    std::vector<Move> pairs;
    pairs.reserve(maxPoints * (maxPoints - 1) / 2);
    for (int i = 0; i < K; i++) {
        if (interrupted()) { out.clear(); return -2; }
        if (stonesRemaining == 1) {
            pairs.emplace_back(pts[i].first, pts[i].first, pts[i].second);
            continue;
        }
        for (int j = i + 1; j < K; j++) {
            if (interrupted()) { out.clear(); return -2; }
            QPoint a = pts[i].first, b = pts[j].first;
            if (m_chess[a.x()][a.y()] != 0 || m_chess[b.x()][b.y()] != 0) continue;
            int sc = evaluatePair(a, b, pieceType);
            pairs.emplace_back(a, b, sc);
        }
    }
    if (interrupted()) { out.clear(); return -2; }
    std::sort(pairs.begin(), pairs.end(),
              [](const Move& a, const Move& b) { return a.score() > b.score(); });
    if (interrupted()) { out.clear(); return -2; }

    int N = std::min((int)pairs.size(), maxPairs);
    out.assign(pairs.begin(), pairs.begin() + N);
    if (out.empty()) out.emplace_back(legal[0], legal[stonesRemaining - 1], 0);
    if (interrupted()) { out.clear(); return -2; }
    return tactic;
}

bool Game::timedOut()
{
    if (!m_timedOut && m_timer.isValid() && m_timer.elapsed() >= m_timeLimitMs)
        m_timedOut = true;
    return m_timedOut;
}

int Game::negamax(int depth, int alpha, int beta, int pieceType, int qply, quint64 hash)
{
    ++m_searchStats.nodes;
    if (timedOut()) return 0;

    const bool useTt = searchConfig.ttEnabled && !searchConfig.threatExtension;
    if (depth > 0 && useTt) {
        ++m_searchStats.ttProbes;
        TTEntry& te = m_tt[hash & (TT_SIZE - 1)];
        if (te.key == hash && te.depth >= depth) {
            if (te.flag == 1 || (te.flag == 2 && te.score >= beta)
                || (te.flag == 3 && te.score <= alpha)) {
                ++m_searchStats.ttHits;
                return te.score;
            }
        }
    }

    int savedType = m_type;
    m_type = pieceType;

    std::vector<Move> cands;
    const bool extend = qply < searchConfig.qMaxPly && m_qNodes < searchConfig.qMaxNodes;
    int tactic = depth == 0 ? tacticalMoves(pieceType, cands, 2, extend) :
                 generateCandidates(pieceType, cands, searchConfig.innerPoints, (depth >= 2 ? searchConfig.innerPairsDeep : searchConfig.innerPairsShallow));
    if (tactic == -2 || timedOut()) {
        m_type = savedType;
        return 0;
    }
    if (tactic == 1 || tactic == -1) {
        m_type = savedType;
        return tactic == 1 ? WIN_SCORE : -WIN_SCORE;
    }

    if (depth == 0 && (tactic != 2 || !extend)) {
        int s = evaluateBoard();
        m_type = savedType;
        return timedOut() ? 0 : std::clamp(s, -WIN_SCORE + 1, WIN_SCORE - 1);
    }
    if (depth == 0) {
        ++m_qNodes;
        for (Move& mv : cands) {
            if (timedOut()) { m_type = savedType; return 0; }
            mv.setScore(evaluatePair(mv.step1(), mv.step2(), pieceType));
        }
        if (timedOut()) { m_type = savedType; return 0; }
        std::sort(cands.begin(), cands.end(),
                  [](const Move& a, const Move& b) { return a.score() > b.score(); });
        if (timedOut()) { m_type = savedType; return 0; }
        if (static_cast<int>(cands.size()) > searchConfig.qMaxDefenses) cands.resize(searchConfig.qMaxDefenses);
    }

    std::vector<QPoint> savedSearch = m_search;
    int oppType = (pieceType == 1) ? 2 : 1;

    if (cands.empty()) {
        int s = evaluateBoard();
        m_type = savedType;
        return timedOut() ? 0 : std::clamp(s, -WIN_SCORE + 1, WIN_SCORE - 1);
    }

    const int originalAlpha = alpha;
    const ZobristTable& z = zobrist();
    int best = -INF;
    for (const Move& mv : cands) {
        if (timedOut()) break;
        QPoint a = mv.step1(), b = mv.step2();
        if (m_chess[a.x()][a.y()] != 0 || m_chess[b.x()][b.y()] != 0) continue;

        QPoint savedLast = m_last, savedLLast = m_llast;
        placeStone(a.x(), a.y(), pieceType);
        placeStone(b.x(), b.y(), pieceType);
        m_last = a; m_llast = b;

        int score;
        if (hasSix(a.x(), a.y(), pieceType) || hasSix(b.x(), b.y(), pieceType)) {
            score = WIN_SCORE;
        } else {
            if (depth > 1)
                rebuildSearch(false);
            const quint64 childHash = hash ^ z.table[a.x()][a.y()][pieceType]
                                      ^ z.table[b.x()][b.y()][pieceType] ^ z.side;
            int childDepth = std::max(0, depth - 1);
            bool extended = false;
            if (depth > 0 && searchConfig.threatExtension && m_threatBudget > 0
                && m_extensions < searchConfig.threatExtendMax
                && createsThreat(a, b, pieceType)) {
                ++childDepth;
                ++m_extensions;
                --m_threatBudget;
                extended = true;
            }
            score = -negamax(childDepth, -beta, -alpha, oppType,
                             depth == 0 ? qply + 1 : 0, childHash);
            if (extended) --m_extensions;
        }

        clearStone(a.x(), a.y());
        clearStone(b.x(), b.y());
        m_last = savedLast; m_llast = savedLLast;
        if (depth > 1)
            m_search = savedSearch;

        if (timedOut()) break;
        if (score > best) best = score;
        if (score > alpha) alpha = score;
        if (alpha >= beta) break;
    }

    m_type = savedType;
    // Selective continuations are scores, not complete mate proofs.
    const int result = m_timedOut ? 0 : std::clamp(best, -WIN_SCORE + 1, WIN_SCORE - 1);
    if (depth > 0 && useTt && !m_timedOut && result > -WIN_SCORE + 1 && result < WIN_SCORE - 1) {
        TTEntry& te = m_tt[hash & (TT_SIZE - 1)];
        if (te.key != hash || te.depth <= depth) {
            ++m_searchStats.ttStores;
            te.key = hash;
            te.depth = depth;
            te.score = result;
            if (result <= originalAlpha) te.flag = 3;
            else if (result >= beta) te.flag = 2;
            else te.flag = 1;
        }
    }
    return result;
}

void Game::bestPlay(int stonesRemaining)
{
    // Odd steps mean the first stone is already on the board; search only one.
    stonesRemaining = m_step % 2 != 0 ? 1 : std::clamp(stonesRemaining, 1, 2);
    int myPiece = m_type;
    int oppPiece = (myPiece == 1) ? 2 : 1;

    m_timer.start();
    m_timeLimitMs = searchConfig.timeLimitMs;
    m_timedOut = false;
    m_qNodes = 0;
    m_searchStats = SearchStats{};
    m_threatBudget = searchConfig.threatExtendBudget;
    m_extensions = 0;

    int savedType = m_type;
    m_type = myPiece;
    QPoint savedLastG = m_last, savedLLastG = m_llast;

    const std::vector<QPoint> originalSearch = m_search;
    rebuildSearch();
    std::vector<Move> root;
    int rootTactic = generateCandidates(myPiece, root, searchConfig.rootMaxPoints, searchConfig.rootMaxPairs, stonesRemaining);

    m_last = savedLastG; m_llast = savedLLastG;
    m_type = savedType;

    if (root.empty() && m_timedOut) {
        std::vector<QPoint> legal;
        for (int x = 0; x < 19; ++x)
            for (int y = 0; y < 19; ++y)
                if (m_chess[x][y] == 0) legal.emplace_back(x, y);
        if (static_cast<int>(legal.size()) >= stonesRemaining)
            root.emplace_back(legal[0], legal[stonesRemaining - 1], 0);
    }
    if (root.empty()) {
        m_search = originalSearch;
        // No legal turn remains; do not fabricate an occupied point.
        m_chosenMove = Move(QPoint(-1, -1), QPoint(-1, -1), 0);
        m_searchStats.stopReason = "no_legal_turn";
        m_searchStats.elapsedMs = m_timer.elapsed();
        emit stop();
        return;
    }

    std::vector<QPoint> savedSearch = m_search;

    // A finite reply list can refute a move, but cannot certify it safe.
    // Keep witnesses independent of scores, alpha bounds and iterative deepening.
    std::vector<Move> witnesses;
    std::vector<bool> refuted(root.size(), false);
    std::vector<bool> probeCompleted(root.size(), false);
    const int rootCount = static_cast<int>(root.size());
    int probeCount = 0, replyCount = 0, excluded = 0, unknown = 0, secondaryCount = 0;
    const auto notation = [](QPoint p) {
        return QString(QChar('A' + p.y())) + QString::number(19 - p.x());
    };
    bool exactDefenseHandled = false;
    std::vector<Move> exactSafeMoves;
    if (rootTactic == 2) {
        exactDefenseHandled = true;
        const qint64 exactDeadline = std::min(searchConfig.probeBudgetMs, m_timeLimitMs);
        const std::vector<QPoint> exactSearch = m_search;
        const QPoint exactLast = m_last, exactLLast = m_llast;
        const int exactType = m_type;
        std::vector<Move> safe, unknownMoves;
        int exactSafe = 0, exactUnknown = 0, exactRefuted = 0;
        int exactReplies = 0;
        std::sort(root.begin(), root.end(), [&](const Move& lhs, const Move& rhs) {
            return evaluatePair(lhs.step1(), lhs.step2(), myPiece)
                   > evaluatePair(rhs.step1(), rhs.step2(), myPiece);
        });
        for (const Move& candidate : root) {
            if (timedOut() || m_timer.elapsed() >= exactDeadline) {
                unknownMoves.push_back(candidate);
                ++exactUnknown;
                continue;
            }
            const QPoint a = candidate.step1(), b = candidate.step2();
            bool refutedCandidate = false;
            bool complete = true;
            m_type = myPiece;
            placeStone(a.x(), a.y(), myPiece);
            placeStone(b.x(), b.y(), myPiece);
            m_last = a; m_llast = b;

            if (!hasSix(a.x(), a.y(), myPiece) && !hasSix(b.x(), b.y(), myPiece)) {
                struct Window { std::vector<int> holes; };
                const auto windows = [&](int piece, int maxHoles) {
                    std::vector<Window> result;
                    const int directions[4][2] = {{0, 1}, {1, 0}, {1, 1}, {1, -1}};
                    for (int x = 0; x < 19; ++x)
                        for (int y = 0; y < 19; ++y)
                            for (const auto& direction : directions) {
                                const int ex = x + 5 * direction[0], ey = y + 5 * direction[1];
                                if (ex < 0 || ex >= 19 || ey < 0 || ey >= 19) continue;
                                std::vector<int> holes;
                                bool blocked = false;
                                for (int k = 0; k < 6; ++k) {
                                    const int cell = m_chess[x + k * direction[0]][y + k * direction[1]];
                                    if (!cell) holes.push_back((x + k * direction[0]) * 19 + y + k * direction[1]);
                                    else if (cell != piece) { blocked = true; break; }
                                }
                                if (!blocked && static_cast<int>(holes.size()) <= maxHoles)
                                    result.push_back({std::move(holes)});
                            }
                    return result;
                };
                const auto attack = windows(oppPiece, 4);
                const auto counter = windows(myPiece, 2);
                std::array<std::vector<int>, 361> incidence;
                std::vector<int> base;
                for (size_t k = 0; k < attack.size(); ++k) {
                    if (attack[k].holes.size() <= 2) base.push_back(static_cast<int>(k));
                    for (int hole : attack[k].holes) incidence[hole].push_back(static_cast<int>(k));
                }
                std::vector<QPoint> legalReplies;
                for (int x = 0; x < 19; ++x)
                    for (int y = 0; y < 19; ++y)
                        if (!m_chess[x][y]) legalReplies.emplace_back(x, y);
                for (size_t i = 0; i < legalReplies.size() && !refutedCandidate; ++i) {
                    for (size_t j = i + 1; j < legalReplies.size(); ++j) {
                        if (timedOut() || m_timer.elapsed() >= exactDeadline) {
                            complete = false;
                            break;
                        }
                        const QPoint c = legalReplies[i], d = legalReplies[j];
                        ++exactReplies;
                        placeStone(c.x(), c.y(), oppPiece);
                        placeStone(d.x(), d.y(), oppPiece);
                        bool fatal = hasSix(c.x(), c.y(), oppPiece) || hasSix(d.x(), d.y(), oppPiece);
                        if (!fatal) {
                            std::vector<int> affected = base;
                            std::vector<unsigned char> seen(attack.size(), 0);
                            for (int index : affected) seen[index] = 1;
                            for (int point : {c.x() * 19 + c.y(), d.x() * 19 + d.y()})
                                for (int index : incidence[point])
                                    if (!seen[index]) { seen[index] = 1; affected.push_back(index); }
                            std::vector<std::vector<int>> threats;
                            for (int index : affected) {
                                std::vector<int> holes;
                                for (int point : attack[index].holes)
                                    if (point != c.x() * 19 + c.y() && point != d.x() * 19 + d.y())
                                        holes.push_back(point);
                                if (holes.empty()) { fatal = true; break; }
                                if (holes.size() <= 2) threats.push_back(std::move(holes));
                            }
                            bool counterWin = false;
                            if (!fatal)
                                for (const Window& window : counter) {
                                    bool untouched = true;
                                    for (int point : window.holes)
                                        if (point == c.x() * 19 + c.y() || point == d.x() * 19 + d.y()) {
                                            untouched = false; break;
                                        }
                                    if (untouched) { counterWin = true; break; }
                            }
                            if (!fatal && !counterWin) {
                                bool coverable = threats.empty();
                                if (!coverable) for (int firstPoint : threats.front()) {
                                    const std::vector<int>* missed = nullptr;
                                    for (const auto& threat : threats)
                                        if (std::find(threat.begin(), threat.end(), firstPoint) == threat.end()) {
                                            missed = &threat; break;
                                        }
                                    if (!missed) { coverable = true; break; }
                                    for (int secondPoint : *missed) {
                                        bool all = true;
                                        for (const auto& threat : threats)
                                            if (std::find(threat.begin(), threat.end(), firstPoint) == threat.end()
                                                && std::find(threat.begin(), threat.end(), secondPoint) == threat.end()) {
                                                all = false; break;
                                            }
                                        if (all) { coverable = true; break; }
                                    }
                                    if (coverable) break;
                                }
                                fatal = !coverable;
                            }
                        }
                        clearStone(c.x(), c.y());
                        clearStone(d.x(), d.y());
                        if (!complete || timedOut() || m_timer.elapsed() >= exactDeadline) {
                            complete = false;
                            break;
                        }
                        if (fatal) {
                            refutedCandidate = true;
                            break;
                        }
                    }
                }
            }
            clearStone(a.x(), a.y());
            clearStone(b.x(), b.y());
            m_last = exactLast; m_llast = exactLLast;
            m_search = exactSearch;
            m_type = exactType;
            if (refutedCandidate) {
                ++exactRefuted;
            } else if (complete) {
                safe.emplace_back(candidate);
                ++exactSafe;
            } else {
                unknownMoves.emplace_back(candidate);
                ++exactUnknown;
            }
        }
        if (!safe.empty()) {
            std::sort(safe.begin(), safe.end(), [&](const Move& lhs, const Move& rhs) {
                return evaluatePair(lhs.step1(), lhs.step2(), myPiece)
                       > evaluatePair(rhs.step1(), rhs.step2(), myPiece);
            });
            exactSafeMoves = safe;
            root = std::move(safe);
            root.insert(root.end(), unknownMoves.begin(), unknownMoves.end());
        } else if (!unknownMoves.empty()) {
            root = std::move(unknownMoves);
        } else {
            root.resize(1);
            rootTactic = -1;
        }
        unknown = exactUnknown;
        excluded = exactRefuted;
        qDebug() << "Exact defense probe: exactDefenseCandidates:" << rootCount
                 << "safe:" << exactSafe << "unknown:" << exactUnknown
                 << "refuted:" << exactRefuted << "replies:" << exactReplies
                 << "elapsedMs:" << m_timer.elapsed();
    }
    qint64 probeDeadline = std::min(searchConfig.probeDiscoveryMs, m_timeLimitMs);
    const auto probe = [&](const Move& mv, bool witnessOnly) {
        if (timedOut() || m_timer.elapsed() >= probeDeadline) return -2;
        ++probeCount;
        const QPoint a = mv.step1(), b = mv.step2();
        placeStone(a.x(), a.y(), myPiece);
        placeStone(b.x(), b.y(), myPiece);
        int result = 0;
        Move witness;
        if (!hasSix(a.x(), a.y(), myPiece) && !hasSix(b.x(), b.y(), myPiece)) {
            std::vector<Move> status;
            int tactic = tacticalMoves(oppPiece, status, 2, false, probeDeadline);
            if (tactic == 1) {
                result = -1;
                witness = status.front();
            } else if (tactic == -2) {
                result = -2;
            } else {
                const auto checkReply = [&](const Move& reply) {
                    if (timedOut() || m_timer.elapsed() >= probeDeadline) return -2;
                    const QPoint c = reply.step1(), d = reply.step2();
                    if (c == d || m_chess[c.x()][c.y()] || m_chess[d.x()][d.y()]) return 0;
                    ++replyCount;
                    placeStone(c.x(), c.y(), oppPiece);
                    placeStone(d.x(), d.y(), oppPiece);
                    int response = tacticalMoves(myPiece, status, 2, false, probeDeadline);
                    // Own immediate win has precedence inside tacticalMoves.
                    bool fatal = response == -1 || hasSix(c.x(), c.y(), oppPiece)
                                 || hasSix(d.x(), d.y(), oppPiece);
                    clearStone(c.x(), c.y());
                    clearStone(d.x(), d.y());
                    if (response == -2 || timedOut() || m_timer.elapsed() >= probeDeadline)
                        return -2;
                    return fatal ? -1 : 0;
                };
                for (const Move& reply : witnesses) {
                    result = checkReply(reply);
                    if (result != 0) { witness = reply; break; }
                }
                if (result == 0 && !witnessOnly) {
                    // Rank replies on the post-root board: before the root defense,
                    // the opponent may have an immediate win which is now blocked.
                    std::vector<Move> replies;
                    auto search = m_search;
                    rebuildSearch(false);
                    int replyTactic = generateCandidates(oppPiece, replies, searchConfig.probePoints,
                                                         searchConfig.probeReplies, 2, probeDeadline);
                    m_search = std::move(search);
                    if (replyTactic == -2) result = -2;
                    if (static_cast<int>(replies.size()) > searchConfig.probeReplies) replies.resize(searchConfig.probeReplies);
                    if (result == 0)
                        for (const Move& reply : replies) {
                            result = checkReply(reply);
                            if (result != 0) { witness = reply; break; }
                        }
                }
            }
        }
        clearStone(a.x(), a.y());
        clearStone(b.x(), b.y());
        if (result == -1) {
            if (!std::any_of(witnesses.begin(), witnesses.end(), [&](const Move& reply) {
                    return (reply.step1() == witness.step1() && reply.step2() == witness.step2())
                        || (reply.step1() == witness.step2() && reply.step2() == witness.step1());
                })) witnesses.push_back(witness);
            qDebug() << "Root counterexample:" << notation(a) << notation(b)
                     << "reply:" << notation(witness.step1()) << notation(witness.step2())
                     << "elapsedMs:" << m_timer.elapsed();
        }
        return result;
    };
    if (!exactDefenseHandled && rootTactic != 1 && rootTactic != -1) {
        for (size_t i = 0; i < root.size(); ++i) {
            int result = probe(root[i], false);
            probeCompleted[i] = result != -2;
            if (result == -1) { refuted[i] = true; ++excluded; }
            if (result == -2) break;
        }
        // Reserve time to propagate discovered counterexamples to other root
        // groups, including ones not reached by the expensive reply ranking.
        probeDeadline = std::min(searchConfig.probeBudgetMs, m_timeLimitMs);
        if (!witnesses.empty())
            for (size_t i = 0; i < root.size(); ++i) {
                if (refuted[i]) continue;
                int result = probe(root[i], true);
                if (result == -1) { refuted[i] = true; ++excluded; }
                if (result == -2) break;
            }
        unknown = 0;
        for (size_t i = 0; i < root.size(); ++i)
            if (!refuted[i] && !probeCompleted[i]) ++unknown;
        std::vector<Move> survivors;
        for (size_t i = 0; i < root.size(); ++i)
            if (!refuted[i]) survivors.push_back(root[i]);
        if (!survivors.empty()) root = std::move(survivors);
        else if (rootTactic == 2) {
            // All exact threat covers have concrete counterexamples.
            qDebug() << "All required root covers refuted; committing a legal losing move";
            root.resize(1);
            rootTactic = -1;
        } else {
            // Try one ranked secondary batch, using the same witnesses and
            // deadline. Neither a finite check nor an interruption proves safety.
            std::vector<Move> rejected = root, secondary;
            if (!timedOut() && m_timer.elapsed() < probeDeadline) {
                int tactic = generateCandidates(myPiece, secondary, 40, 50,
                                                 stonesRemaining, probeDeadline);
                if (tactic != -2)
                    for (const Move& mv : secondary) {
                        bool known = std::any_of(rejected.begin(), rejected.end(), [&](const Move& old) {
                            return (old.step1() == mv.step1() && old.step2() == mv.step2())
                                || (old.step1() == mv.step2() && old.step2() == mv.step1());
                        });
                        if (known) continue;
                        if (secondaryCount == 25) break;
                        ++secondaryCount;
                        int result = probe(mv, false);
                        if (result == -1) { rejected.push_back(mv); ++excluded; }
                        else {
                            survivors.push_back(mv);
                            if (result == -2) ++unknown;
                        }
                    }
            }
            // If ranking/checking ran out of time, keep a legal unknown outside
            // all refuted batches. Never silently recycle a concrete counterexample.
            for (int i = 0; i < 361 && survivors.empty(); ++i) {
                QPoint a(i / 19, i % 19);
                if (m_chess[a.x()][a.y()]) continue;
                for (int j = stonesRemaining == 1 ? i : i + 1; j < 361; ++j) {
                    QPoint b(j / 19, j % 19);
                    if (m_chess[b.x()][b.y()]) continue;
                    bool known = std::any_of(rejected.begin(), rejected.end(), [&](const Move& mv) {
                        return (mv.step1() == a && mv.step2() == b)
                            || (mv.step1() == b && mv.step2() == a);
                    });
                    if (!known) { survivors.emplace_back(a, b, 0); ++unknown; break; }
                    if (stonesRemaining == 1) break;
                }
            }
            if (!survivors.empty()) root = std::move(survivors);
            else { root.resize(1); rootTactic = -1; }
        }
    }
    qDebug() << "Root precheck: candidates:" << rootCount << "probes:" << probeCount
             << "secondaryCandidates:" << secondaryCount
             << "replies:" << replyCount
             << "excluded:" << excluded << "unknown:" << unknown
             << "elapsedMs:" << m_timer.elapsed();

    m_searchStats.rootCandidates = rootCount;
    m_searchStats.rootSurvivors = static_cast<int>(root.size());
    m_searchStats.rootExcluded = excluded;
    m_searchStats.rootUnknown = unknown;
    m_searchStats.precheckMs = m_timer.elapsed();
    int bestScore = rootTactic == 1 ? WIN_SCORE :
                    rootTactic == -1 ? -WIN_SCORE : -INF;
    Move bestMove = root[0];

    m_type = myPiece;
    const ZobristTable& z = zobrist();
    const quint64 rootHash = boardHash();
    const int maxDepth = searchConfig.maxDepth;
    int completedDepth = 0;
    std::vector<Move> scoredRoot;
    for (int depth = 1; rootTactic != 1 && rootTactic != -1 && depth <= maxDepth; depth++) {
        const quint64 nodesBefore = m_searchStats.nodes;
        const quint64 hitsBefore = m_searchStats.ttHits;
        const int budgetBefore = m_threatBudget;
        const int qBefore = m_qNodes;
        int iterBestScore = -INF;
        Move iterBest = bestMove;
        std::vector<Move> iterScores;
        iterScores.reserve(root.size());

        auto pv = std::find_if(root.begin(), root.end(), [&](const Move& mv) {
            return mv.step1() == bestMove.step1() && mv.step2() == bestMove.step2();
        });
        if (pv != root.end()) std::iter_swap(root.begin(), pv);

        for (const Move& mv : root) {
            if (timedOut()) break;
            QPoint a = mv.step1(), b = mv.step2();
            if (m_chess[a.x()][a.y()] != 0 || m_chess[b.x()][b.y()] != 0) continue;

            QPoint savedLast = m_last, savedLLast = m_llast;
            placeStone(a.x(), a.y(), myPiece);
            if (stonesRemaining == 2) placeStone(b.x(), b.y(), myPiece);
            m_last = a; m_llast = b;

            int score;
            if (hasSix(a.x(), a.y(), myPiece) ||
                (stonesRemaining == 2 && hasSix(b.x(), b.y(), myPiece))) {
                score = WIN_SCORE;
            } else {
                rebuildSearch(false);
                quint64 childHash = rootHash ^ z.table[a.x()][a.y()][myPiece];
                if (stonesRemaining == 2) childHash ^= z.table[b.x()][b.y()][myPiece];
                childHash ^= z.side;
                int childDepth = depth - 1;
                bool extended = false;
                if (searchConfig.threatExtension && m_threatBudget > 0
                    && m_extensions < searchConfig.threatExtendMax
                    && createsThreat(a, b, myPiece)) {
                    ++childDepth;
                    ++m_extensions;
                    --m_threatBudget;
                    extended = true;
                }
                score = -negamax(childDepth, -INF, -iterBestScore, oppPiece, 0, childHash);
                if (extended) --m_extensions;            }

            clearStone(a.x(), a.y());
            if (stonesRemaining == 2) clearStone(b.x(), b.y());
            m_last = savedLast; m_llast = savedLLast;
            m_search = savedSearch;

            if (timedOut()) break;
            iterScores.emplace_back(a, b, score);
            if (score > iterBestScore) {
                iterBestScore = score;
                iterBest = Move(a, b, score);
            }
        }

        m_searchStats.iterations.push_back({depth, !m_timedOut,
            m_searchStats.nodes - nodesBefore, m_searchStats.ttHits - hitsBefore,
            static_cast<int>(iterScores.size()), budgetBefore - m_threatBudget,
            m_qNodes - qBefore, m_timer.elapsed()});
        if (m_timedOut)
            break;

        bestScore = iterBestScore;
        bestMove = iterBest;
        completedDepth = depth;
        scoredRoot = std::move(iterScores);

        // 若已找到必胜走法，提前结束
        if (bestScore >= WIN_SCORE)
            break;

        if (m_timer.elapsed() >= m_timeLimitMs)
            break;
    }

    if (!exactSafeMoves.empty()) {
        const bool selectedSafe = std::any_of(exactSafeMoves.begin(), exactSafeMoves.end(), [&](const Move& mv) {
            return (mv.step1() == bestMove.step1() && mv.step2() == bestMove.step2())
                || (mv.step1() == bestMove.step2() && mv.step2() == bestMove.step1());
        });
        if (!selectedSafe) {
            bestMove = exactSafeMoves.front();
            bestScore = evaluatePair(bestMove.step1(), bestMove.step2(), myPiece);
        }
    }

    if (searchConfig.randomize && exactSafeMoves.empty()
        && rootTactic != 1 && rootTactic != -1 && bestScore < WIN_SCORE
        && scoredRoot.size() > 1) {
        std::sort(scoredRoot.begin(), scoredRoot.end(),
                  [](const Move& a, const Move& b) { return a.score() > b.score(); });
        const int topN = std::min<int>(searchConfig.randomTopN, static_cast<int>(scoredRoot.size()));
        const double temp = searchConfig.randomTemp > 0.0 ? searchConfig.randomTemp : 1.0;
        const int bestSc = scoredRoot[0].score();
        std::vector<double> weights(topN);
        double total = 0.0;
        for (int i = 0; i < topN; ++i) {
            weights[i] = std::exp(double(scoredRoot[i].score() - bestSc) / temp);
            total += weights[i];
        }
        std::uniform_real_distribution<double> dist(0.0, total);
        double r = dist(searchRng());
        double acc = 0.0;
        int pick = 0;
        for (int i = 0; i < topN; ++i) {
            acc += weights[i];
            if (r <= acc) { pick = i; break; }
        }
        bestMove = scoredRoot[pick];
        bestScore = bestMove.score();
    }

    m_searchStats.completedDepth = completedDepth;
    m_searchStats.extensionsUsed = searchConfig.threatExtendBudget - m_threatBudget;
    m_searchStats.elapsedMs = m_timer.elapsed();
    m_searchStats.stopReason = rootTactic == 1 ? "immediate_win" :
        rootTactic == -1 ? "root_loss" : bestScore >= WIN_SCORE ? "winning_score" :
        (m_timedOut || m_searchStats.elapsedMs >= m_timeLimitMs) ? "time_limit" : "max_depth";
    m_type = savedType;
    m_last = savedLastG; m_llast = savedLLastG;
    m_search = originalSearch;
    qDebug() << "Search completedDepth:" << completedDepth << "Score:" << bestScore
             << "excluded:" << excluded << "qNodes:" << m_qNodes
             << "elapsedMs:" << m_timer.elapsed()
             << "move:" << notation(bestMove.step1()) << notation(bestMove.step2());

    if (bestScore != -INF) bestMove.setScore(bestScore);
    m_chosenMove = bestMove;
    std::vector<QPoint> temp;

    set(bestMove.step1());
    add(bestMove.step1(), temp);
    emit chessChange(bestMove.step1(), myPiece);
    if (isEnd()) { emit stop(); whoWin(); return; }
    if (stonesRemaining == 1) {
        swap();
        return;
    }
    set(bestMove.step2());
    add(bestMove.step2(), temp);
    emit chessChange(bestMove.step2(), myPiece);
    if (isEnd()) { emit stop(); whoWin(); return; }
    swap();
}
