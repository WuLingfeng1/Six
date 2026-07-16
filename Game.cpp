#include"Game.h"
#include"Chesswoman.h"
#include"vector"
#include <QMessageBox>
#include <QThread>
using std::vector;

int count=0;
Move m_movelist[100000];//保存所有走法进行归并排序
Move m_target[100000];//排序要用到的


Game::Game()
    :m_chess{{0}}
    ,m_step{0}
    ,m_type{1}
{

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
    vector<QPoint>temp;
    m_search.clear();
    m_chess[x][y]=0;
    for (int i = 0; i < 19; i++)
        for (int j = 0; j < 19; j++)
        {
            if (m_chess[i][j] != 0)
                add({ i,j }, temp);
        }
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

    //测试用，增加算法时间，测试计时器使用
    /*int c;
    for (int i = 0; i < 1000; i++) {
        for (int j = 0; j < 1000; j++) {
            for (int m = 0; m < 1000; m++) {
                c = c + (i + j * m * i);
            }
        }
    }*/
    swap();
    //emit stop();
}

void Game::ourplay()
{
    emit restart(m_type);
    //电脑走的时候鼠标点击失效
    //emit clickoff();
    //this->parent()->findChild<QWidget *>("centralwidget")->setAttribute(Qt::WA_TransparentForMouseEvents,false);
    vector<QPoint>temp;
    Comb(m_movelist, m_search.size());
    qDebug() << "Score:"<<m_movelist[0].score();
    //QThread::sleep(2);
    set(m_movelist[0].step1());
    add(m_movelist[0].step1(), temp);
    emit chessChange(m_movelist[0].step1(),m_type);
    if(m_step==2)
    {
        if (isEnd()) {
            emit stop();
            whoWin();
        }

        swap();
        return;
    }
    set(m_movelist[0].step2());
    add(m_movelist[0].step2(), temp);
    emit chessChange(m_movelist[0].step2(),m_type);
    //swap();

    //测试用，增加算法时间，测试计时器使用
    /*int c;
    for (int i = 0; i < 1000; i++) {
        for (int j = 0; j < 1000; j++) {
            for (int m = 0; m < 1000; m++) {
                c = c + (i + j * m * i);
            }
        }
    }*/
    swap();

    //this->parent()->findChild<QWidget *>("centralwidget")->setAttribute(Qt::WA_TransparentForMouseEvents,true);
    //如果结束了显示结果
    if (isEnd()) {
        emit stop();
        whoWin();
    //emit clickon();
    }
}

bool Game::isEnd()
{
    //-1->white  1->black
    // int end1 = 0, end2 = 0, end3 = 0, end4 = 0;
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
                qDebug() << end1 << " " << end2 << " " << end3 << " " << end4;
                m_win = WIN_WHITE;
                return true;
            }
            if (end1 / 6 > 0 || end2 / 6 > 0 || end3 / 6 > 0 || end4 / 6 > 0) {
                qDebug() << end1 << " " << end2 << " " << end3 << " " << end4;
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
                qDebug() << end2;
                m_win = WIN_WHITE;
                return true;
            }
            if (end2 / 6 > 0) {
                qDebug() << end2;
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
                qDebug() << end1;
                m_win = WIN_WHITE;
                return true;
            }
            if (end1 / 6 > 0) {
                qDebug() << end1;
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
{   //emit stop();
    //emit restart(m_type);
    m_type==1?m_type=2:m_type=1;
    m_step==0?m_step=2:m_step=0;
    emit stop();
    emit restart(m_type);
    //emit stop();
}

void Game::set(QPoint location)
{
    int x = location.x();
    int y = location.y();

    m_chess[x][y] = m_type;
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
        if (y + i >= 0 && y + i <= 18 && x + i >= 0 && x + i <= 18 && x - i >= 0 && x - i <= 18)
        {
            QPoint temp1 = { x + i ,y + i };
            QPoint temp2 = { x - i ,y + i };
            if (m_chess[x + i][y + i] == 0 && !contain(m_search, temp1))
            {
                m_search.push_back(temp1);
                lastAdd.push_back(temp1);
            }
            if (m_chess[x - i][y + i] == 0 && !contain(m_search, temp2))
            {
                m_search.push_back(temp2);
                lastAdd.push_back(temp2);
            }
        }
        /*if (y + i >= 0 && y + i <= 18 && x + i >= 0 && x + i <= 18)
        {
            QPoint temp1 = { x + i ,y + i };
            if (m_chess[x + i][y + i] == 0 && !contain(m_search, temp1))
            {
                m_search.push_back(temp1);
                lastAdd.push_back(temp1);
            }

        }

        if (y + i >= 0 && y + i <= 18 && x - i >= 0 && x - i <= 18)
        {
            QPoint temp2 = { x - i ,y + i };

            if (m_chess[x - i][y + i] == 0 && !contain(m_search, temp2))
            {
                m_search.push_back(temp2);
                lastAdd.push_back(temp2);
            }
        }*/
    }
}

void Game::retract(QPoint location,vector<QPoint>&lastAdd)
{
    int x = location.x();
    int y = location.y();
    m_chess[x][y] = 0;
    m_search.push_back(location);
    if (!lastAdd.empty())
    {
        for (vector<QPoint>::iterator it = lastAdd.begin(); it != lastAdd.end(); ++it)
        {
            m_search.erase(std::remove(m_search.begin(), m_search.end(), *it), m_search.end());
            break;
        }
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
    m_chess[a.x()][a.y()] = m_type;
    m_chess[b.x()][b.y()] = m_type;
}

void Game::unmake(QPoint a, QPoint b)
{
    m_chess[a.x()][a.y()] = 0;
    m_chess[b.x()][b.y()] = 0;
}

void Game::mergeSort(Move list[], int n)
{
    int s = 1;
    while (s < n)
    {
        mergePass(list, m_target, s, n);
        s += s;
        mergePass(m_target, list, s, n);
        s += s;
    }
}

void Game::mergePass(Move list[], Move t[], int s, int n)
{
    int i = 0;

    while (i <= n - 2 * s)
    {
        //合并大小为s的相邻二段子数组
        merge(list, t, i, i + s - 1, i + 2 * s - 1);

        i = i + 2 * s;
    }

    if (i + s < n) //剩余的元素个数小於2s
    {

        merge(list, t, i, i + s - 1, n - 1);

    }
    else
        for (int j = i; j <= n - 1; j++)
            t[j] = list[j];
}

void Game::merge(Move list[], Move t[], int l, int m, int r)
{
    int i = l;
    int j = m + 1;
    int k = l;

    while ((i <= m) && (j <= r))
        if (list[i].score() >= list[j].score())
            t[k++] = list[i++];
        else
            t[k++] = list[j++];

    if (i > m)
        for (int q = j; q <= r; q++)
            t[k++] = list[q];
    else
        for (int q = i; q <= m; q++)
            t[k++] = list[q];
}

void Game::AnalyHH()
{
    int x1 = m_last.x(), y1 = m_last.y();
    int x2 = m_llast.x(), y2 = m_llast.y();
    for (int i = y1 - 5 > 0 ? y1 - 5 : 0; i <= y1 && i + 5 < 19; i++)//横向
    {
        int number = 0;
        for (int j = 0; j < 6; j++)
        {
            if (m_chess[x1][i + j] == 2)
                number += 7;
            else
                number += m_chess[x1][i + j];
        }
        if (number == 0 || (number > 6 && number % 7 != 0))
            continue;
        if (number < 7)
        {
            if (m_type == 1) {
                numberOfMyRoad[number]++;
                if (number == 4)
                    numberOfHHfourConnect[0] += 1;
            } else
                numberOfEnemyRoad[number]++;

        }
        else
        {
            if (m_type == 1)
                numberOfEnemyRoad[number / 7]++;
             else {
                numberOfMyRoad[number / 7]++;
                if (number / 7 == 4)
                    numberOfHHfourConnect[0] += 1;
            }
        }
    }
    for (int i = y2 - 5 > 0 ? y2 - 5 : 0; i <= y2 && i + 5 < 19; i++)//横向
    {
        int number = 0;
        if ((i == y1 || i + 1 == y1 || i + 2 == y1 || i + 3 == y1 || i + 4 == y1 || i + 5 == y1) && x1 == x2)
            continue;
        for (int j = 0; j < 6; j++)
        {
            if (m_chess[x2][i + j] == 2)
                number += 7;
            else
                number += m_chess[x2][i + j];
        }
        if (number == 0 ||( number > 6 && number % 7 != 0))
            continue;
        if (number < 7)
        {
            if (m_type == 1) {
                numberOfMyRoad[number]++;
                if (number == 4)
                    numberOfHHfourConnect[0] += 1;
            } else
                numberOfEnemyRoad[number]++;

        }
        else
        {
            if (m_type == 1)
                numberOfEnemyRoad[number / 7]++;
            else {
                numberOfMyRoad[number / 7]++;
                if(number / 7 == 4)
                    numberOfHHfourConnect[0] += 1;
            }
        }
    }
}

void Game::AnalyVV()
{
    int x1 = m_last.x(), y1 = m_last.y();
    int x2 = m_llast.x(), y2 = m_llast.y();
    for (int i = x1 - 5 > 0 ? x1 - 5 : 0; i <= x1 && i + 5 < 19; i++)//竖向
    {
        int number = 0;
        for (int j = 0; j < 6; j++)
        {
            if (m_chess[i + j][y1] == 2)
                number += 7;
            else
                number += m_chess[i + j][y1];
        }
        if (number == 0 ||( number > 6 && number % 7 != 0))
            continue;
        if (number < 7)
        {
            if (m_type == 1) {
                numberOfMyRoad[number]++;
                if (number == 4)
                    numberOfVVfourConnect[0] += 1;
            }
            else
                numberOfEnemyRoad[number]++;
        }
        else
        {
            if (m_type == 1)
                numberOfEnemyRoad[number / 7]++;
            else {
                numberOfMyRoad[number / 7]++;
                if (number / 7 == 4)
                    numberOfVVfourConnect[0] += 1;
            }
        }
    }
    for (int i = x2 - 5 > 0 ? x2 - 5 : 0; i <= x2 && i + 5 < 19; i++)//竖向
    {
        int number = 0;
        if ((i == x1 || i + 1 == x1 || i + 2 == x1 || i + 3 == x1 || i + 4 == x1 || i + 5 == x1) && y1 == y2)
            continue;
        for (int j = 0; j < 6; j++)
        {
            if (m_chess[i + j][y2] == 2)
                number += 7;
            else
                number += m_chess[i + j][y2];
        }
        if (number == 0 ||( number > 6 && number % 7 != 0))
            continue;
        if (number < 7)
        {
            if (m_type == 1) {
                numberOfMyRoad[number]++;
                if (number == 4)
                    numberOfVVfourConnect[0] += 1;
            }
            else
                numberOfEnemyRoad[number]++;
        }
        else
        {
            if (m_type == 1)
                numberOfEnemyRoad[number / 7]++;
            else {
                numberOfMyRoad[number / 7]++;
                if (number == 4)
                    numberOfVVfourConnect[0] += 1;
            }
        }
    }
}

void Game::AnalyRR()
{
    int x1 = m_last.x(), y1 = m_last.y();
    int x2 = m_llast.x(), y2 = m_llast.y();
    int i, j;
    if (x1 - 5 < 0 || y1 - 5 < 0)
    {
        int t = std::min(x1, y1);
        i = x1 - t;
        j = y1 - t;
    }
    else
    {
        i = x1 - 5;
        j = y1 - 5;
    }
    for (; i <= x1 && i + 5 < 19 && j <= y1 && j + 5 < 19; i++, j++)//左向
    {
        int number = 0;
        for (int k = 0; k < 6; k++)
        {
            if (m_chess[i + k][j + k] == 2)
                number += 7;
            else
                number += m_chess[i + k][j + k];
        }
        if (number == 0 || (number > 6 && number % 7 != 0))
            continue;
        if (number < 7)
        {
            if (m_type == 1) {
                numberOfMyRoad[number]++;
                if (number == 4)
                    numberOfRRfourConnect[0] += 1;
            }
            else
                numberOfEnemyRoad[number]++;
        }
        else
        {
            if (m_type == 1)
                numberOfEnemyRoad[number / 7]++;
            else {
                numberOfMyRoad[number / 7]++;
                if (number / 7 == 4)
                    numberOfRRfourConnect[0] += 1;
            }
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
    for (; i <= x2 && i + 5 < 19 && j <= y2 && j + 5 < 19; i++, j++)//左向
    {
        int number = 0;
        if ((i == x1 && j == y1 )|| (i + 1 == x1 && j + 1 == y1 )||( i + 2 == x1 && j + 2 == y1) || (i + 3 == x1 && j + 3 == y1) || (i + 4 == x1 && j + 4 == y1 )|| (i + 5 == x1 && j + 5 == y1))
            continue;
        for (int k = 0; k < 6; k++)
        {
            if (m_chess[i + k][j + k] == 2)
                number += 7;
            else
                number += m_chess[i + k][j + k];
        }
        if (number == 0 ||( number > 6 && number % 7 != 0))
            continue;
        if (number < 7)
        {
            if (m_type == 1) {
                numberOfMyRoad[number]++;
                if(number == 4)
                    numberOfRRfourConnect[0] += 1;
            }
            else
                numberOfEnemyRoad[number]++;
        }
        else
        {
            if (m_type == 1)
                numberOfEnemyRoad[number / 7]++;
            else {
                numberOfMyRoad[number / 7]++;
                if(number / 7 == 4)
                    numberOfRRfourConnect[0] += 1;
            }
        }
    }
}

void Game::AnalyLL()
{
    int x1 = m_last.x(), y1 = m_last.y();
    int x2 = m_llast.x(), y2 = m_llast.y();
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
    for (;
         i <= x1 && i + 5 < 19 && j >= y1 && j - 5 >= 0; i++, j--)//右向
    {
        int number = 0;
        for (int k = 0; k < 6; k++)
        {
            if (m_chess[i + k][j - k] == 2)
                number += 7;
            else
                number += m_chess[i + k][j - k];
        }
        if (number == 0 ||(number > 6 && number % 7 != 0))
            continue;
        if (number < 7)
        {
            if (m_type == 1) {
                numberOfMyRoad[number]++;
                if (number == 4)
                    numberOfLLfourConnect[0] += 1;
            }
            else
                numberOfEnemyRoad[number]++;
        }
        else
        {
            if (m_type == 1)
                numberOfEnemyRoad[number / 7]++;
            else {
                numberOfMyRoad[number / 7]++;
                if (number / 7 == 4)
                    numberOfLLfourConnect[0] += 1;
            }
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
    for (;
         i <= x2 && i + 5 < 19 && j >= y2 && j - 5 >= 0; i++, j--)//右向
    {
        int number = 0;
        if ((i == x1 && j == y1 )|| (i + 1 == x1 && j - 1 == y1) || (i + 2 == x1 && j - 2 == y1) ||( i + 3 == x1 && j - 3 == y1) || (i + 4 == x1 && j - 4 == y1 )|| (i + 5 == x1 && j - 5 == y1))
            continue;
        for (int k = 0; k < 6; k++)
        {
            if (m_chess[i + k][j - k] == 2)
                number += 7;
            else
                number += m_chess[i + k][j - k];
        }
        if (number == 0 || (number > 6 && number % 7 != 0))
            continue;
        if (number < 7)
        {
            if (m_type == 1) {
                numberOfMyRoad[number]++;
                if(number == 4)
                    numberOfLLfourConnect[0] += 1;
            }
            else
                numberOfEnemyRoad[number]++;
        }
        else
        {
            if (m_type == 1)
                numberOfEnemyRoad[number / 7]++;
            else {
                numberOfMyRoad[number / 7]++;
                if (number / 7 ==4)
                    numberOfLLfourConnect[0] += 1;
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
        if (numberOfHHfourConnect[0] > 0) {
            for (int i = y1 - 5 > 0 ? y1 - 5 : 0; i <= y1 && i + 7 < 19; i++) {
                if (m_chess[x1][i] == m_chess[x1][i + 1] && m_chess[x1][i + 1] == m_chess[x1][i + 6]
                    && m_chess[x1][i + 6] == m_chess[x1][i + 7] && m_chess[x1][i + 7] == 0
                    && m_chess[x1][i + 2] == m_chess[x1][i + 3] && m_chess[x1][i + 3] == m_chess[x1][i + 4]
                    && m_chess[x1][i + 4] == m_chess[x1][i + 5]
                    && (m_chess[x1][i + 5] == 1 || m_chess[x1][i + 5] == 2)) {
                    numberOfHHfourConnect[1] += 1;
                    qDebug() << "cunzaihuosiHH1";
                }
            }
            for (int i = y2 - 5 > 0 ? y2 - 5 : 0; i <= y2 && i + 7 < 19; i++) {
                if (m_chess[x2][i] == m_chess[x2][i + 1] && m_chess[x2][i + 1] == m_chess[x2][i + 6]
                    && m_chess[x2][i + 6] == m_chess[x2][i + 7] && m_chess[x2][i + 7] == 0
                    && m_chess[x2][i + 2] == m_chess[x2][i + 3] && m_chess[x2][i + 3] == m_chess[x2][i + 4]
                    && m_chess[x2][i + 4] == m_chess[x2][i + 5]
                    && (m_chess[x2][i + 5] == 1 || m_chess[x2][i + 5] == 2)) {
                    numberOfHHfourConnect[1] += 1;
                    qDebug() << "cunzaihuosiHH2";
                }
            }
        }

        if (numberOfVVfourConnect[0] > 0) {
            for (int i = x1 - 5 > 0 ? x1 - 5 : 0; i <= x1 && i + 7 < 19; i++) {
                if (m_chess[i][y1] == m_chess[i + 1][y1] && m_chess[i + 1][y1] == m_chess[i + 6][y1]
                    && m_chess[i + 6][y1] == m_chess[i + 7][y1] && m_chess[i + 7][y1] == 0
                    && m_chess[i + 2][y1] == m_chess[i + 3][y1] && m_chess[i + 3][y1] == m_chess[i + 4][y1]
                    && m_chess[i + 4][y1] == m_chess[i + 5][y1]
                    && (m_chess[i + 5][y1] == 1 || m_chess[i + 5][y1] == 2)) {
                    numberOfVVfourConnect[1] += 1;
                    qDebug() << "cunzaihuosiVV1";
                }
            }
            for (int i = x2 - 5 > 0 ? x2 - 5 : 0; i <= x2 && i + 7 < 19; i++) {
                if (m_chess[i][y2] == m_chess[i + 1][y2] && m_chess[i + 1][y2] == m_chess[i + 6][y2]
                    && m_chess[i + 6][y2] == m_chess[i + 7][y2] && m_chess[i + 7][y2] == 0
                    && m_chess[i + 2][y2] == m_chess[i + 3][y2] && m_chess[i + 3][y2] == m_chess[i + 4][y2]
                    && m_chess[i + 4][y2] == m_chess[i + 5][y2]
                    && (m_chess[i + 5][y2] == 1 || m_chess[i + 5][y2] == 2)) {
                    numberOfVVfourConnect[1] += 1;
                    qDebug() << "cunzaihuosiVV2";
                }
            }
        }

        if (numberOfRRfourConnect[0] > 0) {
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
                    numberOfRRfourConnect[1] += 1;
                    qDebug() << "cunzaihuosiRR1";
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
                    numberOfRRfourConnect[1] += 1;
                    qDebug() << "cunzaihuosiRR2";
                }
            }
        }

        if (numberOfLLfourConnect[0] > 0) {
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
                    numberOfLLfourConnect[1] += 1;
                    qDebug() << "cunzaihuosiLL1";
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
                    numberOfLLfourConnect[1] += 1;
                    qDebug() << "cunzaihuosiLL2";
                }
            }
        }
    }
    //else {
    //    qDebug() << "无连四棋型";
    //}
}

bool Game::CheckWinningPatterns()
{
    int number = 0;
    int HH = 0;
    int VV = 0;
    int RR = 0;
    int LL = 0;
    if (numberOfHHfourConnect[1] > 0) {
        number++;
        HH++;
    }
    if (numberOfVVfourConnect[1] > 0) {
        number++;
        VV++;
    }
    if (numberOfRRfourConnect[1] > 0) {
        number++;
        RR++;
    }
    if (numberOfLLfourConnect[1] > 0) {
        number++;
        LL++;
    }
    if (number >= 2) return true;

    if (number == 1) {
        if (HH > 0 && (numberOfVVfourConnect[0] > 0 || numberOfRRfourConnect[0] > 0 || numberOfLLfourConnect[0] > 0))
            return true;
        if (VV > 0 && (numberOfHHfourConnect[0] > 0 || numberOfRRfourConnect[0] > 0 || numberOfLLfourConnect[0] > 0))
            return true;
        if (RR > 0 && (numberOfVVfourConnect[0] > 0 || numberOfHHfourConnect[0] > 0 || numberOfLLfourConnect[0] > 0))
            return true;
        if (LL > 0 && (numberOfHHfourConnect[0] > 0 || numberOfRRfourConnect[0] > 0 || numberOfVVfourConnect[0] > 0))
            return true;
    }
    return false;
}

void Game::resetFourVariable()
{
    for(int i = 0;i<2;i++) {
    numberOfHHfourConnect[i] = 0;
    numberOfLLfourConnect[i] = 0;
    numberOfRRfourConnect[i] = 0;
    numberOfVVfourConnect[i] = 0;
    numberOfFourConnect[i] = 0;
    }
}

int Game::estimate()
{
    int pre = 0, after = 0;
    int x1 = m_last.x(), y1 = m_last.y();
    int x2 = m_llast.x(), y2 = m_llast.y();
    int type = m_chess[x1][y1];
    m_chess[x1][y1] = 0;//清除落子
    m_chess[x2][y2] = 0;//清除落子
    resetFourVariable();
    for (int i = 0; i < 7; i++)
    {
        numberOfMyRoad[i] = 0;
        numberOfEnemyRoad[i] = 0;
    }
    AnalyHH();
    AnalyVV();
    AnalyRR();
    AnalyLL();
    //qDebug() << "HH" << numberOfHHfourConnect << "VV" << numberOfVVfourConnect << "LL" << numberOfLLfourConnect << "RR"
    //         << numberOfRRfourConnect;
    //AnalyFourConnect();
    for (int i = 1; i <= 6; i++)
        pre += numberOfMyRoad[i] * scoreOfMyRoad[i] - numberOfEnemyRoad[i] * scoreOfEnemyRoad[i];
    resetFourVariable();
    m_chess[x1][y1] = type;
    m_chess[x2][y2] = type;
    for (int i = 0; i < 7; i++)
    {
        numberOfMyRoad[i] = 0;
        numberOfEnemyRoad[i] = 0;
    }
    AnalyHH();
    AnalyVV();
    AnalyRR();
    AnalyLL();
    AnalyFourConnect();
    for (int i = 1; i <= 6; i++)
        after += numberOfMyRoad[i] * scoreOfMyRoad[i] - numberOfEnemyRoad[i] * scoreOfEnemyRoad[i];
    if (CheckWinningPatterns() == true)
        after += scoreOfWinningChessPattern[1];
    resetFourVariable();
    return after - pre;
}
bool hasDone(vector<bool>index, int n, int k)
{
    for (int i = n - 1; i >= n - k; i--)
    {
        if (!index[i])
        {
            return false;
        }
    }
    return true;
}
void Game::Comb(Move list[], int n)
{
    QPoint temp[2];
    int score;
    m_count = 0;
    vector <bool>index(n, false);
    //选中前k个位置
    for (int i = 0, j = 0; i < 2; i++)
    {
        index[i] = true;
        temp[j++] = m_search[i];
    }
    make(temp[0], temp[1]);
    score = estimate();
    list[m_count++] = Move(temp[0],temp[1],score);
    unmake(temp[0], temp[1]);
    while (!hasDone(index, n, 2))
    {
        //从左到右扫描数组
        for (int i = 0; i < n - 1; i++)
        {
            //找到第一个“10”组合将其变成"01"组合
            if (index[i] && !index[i + 1])
            {
                index[i] = false;
                index[i + 1] = true;

                //将"01"组合左边的1移到最左边
                int count = 0;
                for (int j = 0; j < i; j++)
                {
                    if (index[j])
                    {
                        index[j] = false;
                        index[count++] = true;
                    }
                }
                for (int i = 0, j = 0; i < n; i++)
                {
                    if (index[i])
                        temp[j++] = m_search[i];
                }
                make(temp[0], temp[1]);
                score = estimate();
                if (score >= 80000)
                {
                    m_movelist[0] = { temp[0],temp[1],score };
                    unmake(temp[0], temp[1]);
                    return;
                }
                else
                {
                    list[m_count++] = { temp[0],temp[1],score };
                    unmake(temp[0], temp[1]);
                }
                break;
            }
        }
    }
    mergeSort(list, m_count);
}

bool Game::contain(std::vector<QPoint>m_search, QPoint value)
{

    for (vector<QPoint>::iterator it = m_search.begin(); it != m_search.end(); it++)
    {
        if ((*it) == value)
            return true;
    }
    return false;
}

