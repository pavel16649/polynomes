#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <vector>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

struct node {
    node* prev;
    node* next;
    int multiplier;
    std::vector <int> variables;

    node() {
        variables.resize(26, 0);
    }
};

struct node_base {
    node_base* prev;
    node_base* next;
    node* head;
    node* tail;
//    std::vector <int> variables;

//    node_base() {
//        variables.resize(26, 0);
//    }
//    ~node_base() {
//        node* ptr = this->head;
//        if (ptr == nullptr) return;
//        while (ptr->next != nullptr) {
//            ptr = ptr->next;
//            delete ptr->prev;
//        }
//        delete ptr;
//    }
};

enum { A = 0, B, C, D, E, F };
const double POWER = 1.0, SIGN = 2.0, VARIABLE = 3.0, DIGIT = 4.0;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void add_to_table(node_base* ptr_base, int mode);

private slots:
    void on_add_clicked();

    bool state_machine(QString input, int mode);

    void add_from_textbox(QString input);

    void on_der_clicked();

    void on_plus_clicked();

    void on_roots_clicked();

    void on_del_clicked();

    void on_mul_clicked();

    void mul(node_base* left, node_base* right, int mode = 1);

    void plus(node_base* left, node_base* right);

    void minus(node_base* left, node_base* right, int mode = 1);

    void find_der(node_base* ptr_base, const int &var, int por);

    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void on_der_2_clicked();

    void on_minus_clicked();

    void value(node_base* ptr_base);

    void on_div_clicked();

    void division(node_base* left, node_base* right);

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
