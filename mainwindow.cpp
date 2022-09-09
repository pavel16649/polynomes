#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <cmath>
#include <QtGui>
#include <QMessageBox>
#include <fstream>
#include <QInputDialog>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->tableWidget->setColumnCount(1);

    QRegExp numbers("[1-9]+");
    ui->del_num->setValidator(new QRegExpValidator(numbers, this));
    ui->number->setValidator(new QRegExpValidator(numbers, this));
    ui->left->setValidator(new QRegExpValidator(numbers, this));
    ui->right->setValidator(new QRegExpValidator(numbers, this));
    ui->der_por->setValidator(new QRegExpValidator(numbers, this));
    QRegExp letters("[a-z]");
    ui->der_var->setValidator(new QRegExpValidator(letters, this));
}

MainWindow::~MainWindow()
{
    delete ui;
}

node_base* head = nullptr;
node_base* tail = nullptr;

bool MainWindow::state_machine(QString input, int mode = 0)
{
    int pos = 0;
    try {
        if (input == "") throw -1;
        int state = A;
        for ( ; pos < input.size(); ++pos) {
            QChar ch = input[pos];
            if (ch == " ") {
                continue;
            } else if (ch.unicode() > 0x0400 && ch.unicode() < 0x04FF) {
                throw ch;
            } else if (state == A && ch.isDigit()) {
                state = B;
            } else if (state == A && ch.isLetter()) {
                state = C;
            } else if (state == A && (ch == "-" || ch == "+")) {
                state = D;
            } else if (state == A && ch == "^") {
                throw POWER;
            } else if (state == B && ch.isDigit()) {
                state = B;
            } else if (state == B && ch.isLetter()) {
                state = C;
            } else if (state == B && (ch == "-" || ch == "+")) {
                state = D;
            } else if (state == B && ch == "^") {
                throw POWER;
            } else if (state == C && ch.isLetter()) {
                state = C;
            } else if (state == C && (ch == "-" || ch == "+")) {
                state = D;
            } else if (state == C && ch == "^") {
                state = E;
            } else if (state == C && ch.isDigit()) {
                throw DIGIT;
            } else if (state == D && ch.isDigit()) {
                state = B;
            } else if (state == D && ch.isLetter()) {
                state = C;
            } else if (state == D && ch == "^") {
                throw POWER;
            } else if (state == D && (ch == "-" || ch == "+")) {
                throw SIGN;
            } else if (state == E && ch.isDigit()) {
                state = F;
            } else if (state == E && (ch == "-" || ch == "+")) {
                throw SIGN;
            } else if (state == E && ch.isLetter()) {
                throw VARIABLE;
            } else if (state == E && ch == "^") {
                throw POWER;
            } else if (state == F && (ch == "-" || ch == "+")) {
                state = D;
            } else if (state == F && ch.isDigit()) {
                state = F;
            } else if (state == F && ch.isLetter()) {
                state = C;
            } else if (state == F && ch == "^") {
                throw POWER;
            } else {
                throw ch;
            }
        }
        if (!(state == C || state == F || state == B)) {
            throw 0;
        }
        return true;

    } catch (int err) {
        if (err == -1 && !mode) {
            QMessageBox msgBox(QMessageBox::Warning,
                               QString::fromUtf8("Error!"),
                               QString::fromUtf8("Input is empty!"));
            msgBox.exec();
        } else if (err == 0 && !mode) {
            QMessageBox msgBox(QMessageBox::Warning,
                               QString::fromUtf8("Error!"),
                               QString::fromUtf8("Input is incomplete!"));
            msgBox.exec();
        }
        return false;
    } catch (QChar ch) {
        if (mode) return false;
        QString out("Invalid symbol ");
        out += ch;
        out += " in position ";
        out += QString::number(++pos);
        QMessageBox msgBox(QMessageBox::Warning,
                           QString::fromUtf8("Error!"),
                           out);
        msgBox.exec();
        return false;
    } catch (double ch) {
        if (mode) return false;
        QString out("Unexpected symbol ");
        out += input[pos];
        out += " in position ";
        out += QString::number(++pos);
        QMessageBox msgBox(QMessageBox::Warning,
                           QString::fromUtf8("Error!"),
                           out);
        msgBox.exec();
        return false;
    }
}

inline node_base* find_in_base(int ind_table)
{
    node_base* ptr_base = head;
    for (int i = 1; i < ind_table; ++i) {
        ptr_base = ptr_base->next;
    }
    return ptr_base;
}

QString construct_polynome(node_base* ptr_base)
{
    QString res("");
    node* ptr = ptr_base->head;
    if (ptr->multiplier == 0) {
        return "0";
    }

    while (ptr != nullptr) {
        if (ptr->multiplier > 0) {
            res += "+";
        } else {
            res += "-";
        }
        bool chk = true;
        for (int i = 0; i < 26; ++i) {
            if (ptr->variables[i] != 0) {
                chk = false;
                break;
            }
        }
        if (chk) {
            res += QString::number(abs(ptr->multiplier));
            ptr = ptr->next;
            continue;
        }
        if (abs(ptr->multiplier) != 1) {
            res += QString::number(abs(ptr->multiplier));
        }
        for (int i = 0; i < 26; ++i) {
            if (ptr->variables[i] == 0) continue;

            QChar tmp(97 + i);
            res += tmp;
            if (ptr->variables[i] > 1) {
                res += "^";
                res += QString::number(ptr->variables[i]);
            }
        }

        ptr = ptr->next;
    }
    if (res[0] == "+") {
        res = res.mid(1);
    }
    return res;
}

bool check_inclusion(const int& multiplier, const std::vector <int> &variables, node_base* ptr_base = tail)
{
    if (ptr_base->head == nullptr) return false;
    node* ptr = ptr_base->head;
    if (ptr->multiplier == 0) return false;
    while (ptr != nullptr) {
        if (variables == ptr->variables) {
            ptr->multiplier += multiplier;
            if (ptr->multiplier == 0) {
                if (ptr->prev == nullptr) {
                    if (ptr->next == nullptr) {
                        delete ptr;
                        ptr_base->head = nullptr;
                        ptr_base->tail = nullptr;
                    } else {
                        ptr->next->prev = nullptr;
                        ptr_base->head = ptr->next;
                        delete ptr;
                    }
                } else if (ptr->next == nullptr) {
                    ptr_base->tail = ptr->prev;
                    ptr_base->tail->next = nullptr;
                    delete ptr;
                } else {
                    ptr->prev->next = ptr->next;
                    ptr->next->prev = ptr->prev;
                    delete ptr;
                }
            }

            if (ptr_base->head == nullptr) {
                node* new_head = new node;
                new_head->prev = nullptr;
                new_head->next = nullptr;
                new_head->multiplier = 0;
                for (int i = 0; i < 26; ++i) {
                    new_head->variables[i] = 0;
                }
                ptr_base->head = new_head;
                ptr_base->tail = new_head;
            }
            return true;
        }
        ptr = ptr->next;
    }
    return false;
}

void clear_tmp(int &multiplier, std::vector <int> &variables, QString &res, QChar &ch)
{
    for (int i = 0; i < 26; ++i) {
        variables[i] = 0;
    }
    if (ch == "-") {
        multiplier = -1;
    } else if (ch == "+"){
        multiplier = 1;
    }
    res = "";
}

void add_polynome(node_base* New = nullptr)
{
    if (New == nullptr) {
        New = new node_base;
        New->prev = nullptr;
        New->next = nullptr;
        New->head = nullptr;
        New->tail = nullptr;
    }
    if (head == nullptr) {
        head = New;
        tail = New;
        return;
    }
    tail->next = New;
    New->prev = tail;
    tail = New;
}

void MainWindow::add_to_table(node_base* ptr_base, int mode = 1)
{
    if (!mode) {
        add_polynome(ptr_base);
        return;
    }
    QMessageBox msgBox;
    msgBox.setText("Message");
    QString res("Do you want to add this polynome to base?\n");
    res += construct_polynome(ptr_base);
    msgBox.setInformativeText(res);
    msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Cancel);
    msgBox.setIcon(QMessageBox::Information);
    int ret = msgBox.exec();
    if (ret != 0x00400000) {
        add_polynome(ptr_base);
        ui->tableWidget->insertRow(ui->tableWidget->rowCount());
        ui->tableWidget->setItem(ui->tableWidget->rowCount()-1, 0,
                                 new QTableWidgetItem(construct_polynome(tail)));
    }
}

void del_polynome(node_base* ptr_base)
{
    if (ptr_base->prev == nullptr) {
        if (ptr_base->next == nullptr) {
            delete ptr_base;
            head = nullptr;
            tail = nullptr;
        } else {
            ptr_base->next->prev = nullptr;
            head = ptr_base->next;
            delete ptr_base;
        }
    } else if (ptr_base->next == nullptr) {
        tail = ptr_base->prev;
        tail->next = nullptr;
        delete ptr_base;
    } else {
        ptr_base->prev->next = ptr_base->next;
        ptr_base->next->prev = ptr_base->prev;
        delete ptr_base;
    }
}



void add_monomial(const int &multiplier, const std::vector <int> &variables, node_base* ptr_base = tail)
{
    if (ptr_base->head != nullptr && multiplier == 0) return;
    node* new_node = new node;
    new_node->multiplier = multiplier;
    new_node->variables = variables;
    if (ptr_base->head != nullptr) {
        if (ptr_base->head->multiplier == 0) {
            delete ptr_base->head;
            ptr_base->head = nullptr;
            ptr_base->tail = nullptr;
        }
    }
    if (ptr_base->head == nullptr) {
        ptr_base->head = new_node;
        ptr_base->tail = new_node;
        new_node->next = nullptr;
        new_node->prev = nullptr;
        return;
    }
    new_node->prev = ptr_base->tail;
    new_node->next = nullptr;
    if (new_node->prev != nullptr) {
        new_node->prev->next = new_node;
    }

    ptr_base->tail = new_node;
}

void MainWindow::add_from_textbox (QString input)
{
    add_polynome();
    int pos = 0;
    int tmp_mul = 1;
    std::vector <int> tmp_vars;
    QString res = "";
    tmp_vars.resize(26, 0);
    for ( ; pos < input.size(); ++pos) {
        QChar ch = input[pos];
        if (ch == " ") {
            continue;
        } else if (ch == "-" || ch == "+") {
            if (res.isEmpty()) {
                if (ch == "-") {
                    tmp_mul = -1;
                } else if (ch == "+"){
                    tmp_mul = 1;
                }
                continue;
            }

            int i = 0;
            while (res[i].isDigit()) {
                ++i;
            }
            if (i == res.size()) {
                tmp_mul *= res.leftRef(i).toInt();
                if (!check_inclusion(tmp_mul, tmp_vars)) {
                    add_monomial(tmp_mul, tmp_vars);
                }
                clear_tmp(tmp_mul, tmp_vars, res, ch);
                continue;
            }
            if (i > 0) {
                tmp_mul *= res.leftRef(i).toInt();
            } else {
                if (res.size() == 1) {
                    ++tmp_vars[res[0].unicode() - 97];
                    if (!check_inclusion(tmp_mul, tmp_vars)) {
                        add_monomial(tmp_mul, tmp_vars);
                    }
                    clear_tmp(tmp_mul, tmp_vars, res, ch);
                    continue;
                }
            }

            bool flag_pow = false;
            QChar var(res[i]);
            int ind_from = -1;
            for (++i ; i < res.size(); ++i) {
                if (res[i].isLetter() && !flag_pow) {
                    if (i > 0) {
                        if (res[i-1].isLetter()) {
                            ++tmp_vars[var.unicode() - 97];
                        }
                    }
                    var = res[i];
                } else if (res[i] == "^") {
                    ind_from = i + 1;
                    flag_pow = true;
                } else if (res[i].isLetter() && flag_pow) {
                    tmp_vars[var.unicode() - 97] += res.midRef(ind_from, i - ind_from).toInt();
                    flag_pow = false;
                    var = res[i];
                }
            }
            if (flag_pow) {
                tmp_vars[var.unicode() - 97] += res.rightRef(res.size() - ind_from).toInt();
            } else {
                ++tmp_vars[var.unicode() - 97];
            }
            if (!check_inclusion(tmp_mul, tmp_vars)) {
                add_monomial(tmp_mul, tmp_vars);
            }
            clear_tmp(tmp_mul, tmp_vars, res, ch);

        } else {
            res += ch;
        }
    }
    if (!res.isEmpty()) {
        int i = 0;
        while (res[i].isDigit()) {
            ++i;
        }
        if (i == res.size()) {
            tmp_mul *= res.leftRef(i).toInt();
            if (!check_inclusion(tmp_mul, tmp_vars)) {
                add_monomial(tmp_mul, tmp_vars);
            }
            return;
        }
        if (i > 0) {
            tmp_mul *= res.leftRef(i).toInt();
        } else {
            if (res.size() == 1) {
                ++tmp_vars[res[0].unicode() - 97];
                if (!check_inclusion(tmp_mul, tmp_vars)) {
                    add_monomial(tmp_mul, tmp_vars);
                }
                return;
            }
        }
        bool flag_pow = false;
        QChar var(res[i]);
        int ind_from = -1;
        for (++i ; i < res.size(); ++i) {
            if (res[i].isLetter() && !flag_pow) {
                if (i > 0) {
                    if (res[i-1].isLetter()) {
                        ++tmp_vars[var.unicode() - 97];
                    }
                }
                var = res[i];
            } else if (res[i] == "^") {
                ind_from = i + 1;
                flag_pow = true;
            } else if (res[i].isLetter() && flag_pow) {
                tmp_vars[var.unicode() - 97] += res.midRef(ind_from, i - ind_from).toInt();
                flag_pow = false;
                var = res[i];
            }
        }
        if (flag_pow) {
            tmp_vars[var.unicode() - 97] += res.rightRef(res.size() - ind_from).toInt();
        } else {
            ++tmp_vars[var.unicode() - 97];
        }
        if (!check_inclusion(tmp_mul, tmp_vars)) {
            add_monomial(tmp_mul, tmp_vars);
        }
    }

}

void MainWindow::on_add_clicked()
{

    QString input = ui->lineEdit->text();
    if (!state_machine(input)) return;
    add_from_textbox(input);
    ui->tableWidget->insertRow(ui->tableWidget->rowCount());
    ui->tableWidget->setItem(ui->tableWidget->rowCount()-1, 0,
                             new QTableWidgetItem(construct_polynome(tail)));
}

void MainWindow::find_der(node_base* ptr_base, const int &var, int por)
{
    if (ptr_base->head->multiplier == 0) {
        add_to_table(ptr_base);
        return;
    }
    if (por == 0) {
        add_to_table(ptr_base);
        return;
    }
    node_base* der = new node_base;
    if (ptr_base->head->multiplier == 0) {
        node* derivative = new node;
        derivative->multiplier = 0;
        derivative->next = nullptr;
        derivative->prev = nullptr;
        der->head = derivative;
        der->tail = derivative;
    } else {
        node* ptr = ptr_base->head;
        der->prev = nullptr;
        der->next = nullptr;
        der->head = nullptr;
        der->tail = nullptr;
        while (ptr != nullptr) {
            if (ptr->variables[var] > 0) {
                int multiplier = ptr->multiplier * ptr->variables[var];
                --(ptr->variables[var]);
                if (!check_inclusion(multiplier, ptr->variables, der)) {
                    add_monomial(multiplier, ptr->variables, der);
                }
                ++(ptr->variables[var]);
            } else {

            }
            ptr = ptr->next;
        }
        if (der->head == nullptr) {
            node* derivative = new node;
            derivative->multiplier = 0;
            derivative->next = nullptr;
            derivative->prev = nullptr;
            der->head = derivative;
            der->tail = derivative;
        }
    }
    find_der(der, var, por - 1);
    node* ptr = der->head;
    while (ptr != 0) {
        node* tmp = ptr;
        delete tmp;
        ptr = ptr->next;
    }
    delete der;
}

void MainWindow::on_der_clicked()
{
    try {
        int num = ui->number->text().toInt();
        int por = ui->der_por->text().toInt();
        if (num > ui->tableWidget->rowCount()) {
            throw -1;
        }

        QChar var = (ui->der_var->text())[0];
        node_base* ptr_base = find_in_base(num);
        find_der(ptr_base, var.unicode() - 97, por);

    }  catch (int err) {
        QMessageBox msgBox(QMessageBox::Warning,
                           QString::fromUtf8("Error!"),
                           QString::fromUtf8("There is no such index in the table!"));
        msgBox.exec();
    }
}

void MainWindow::plus(node_base* left, node_base* right)
{
    node_base* sum = new node_base;
    sum->prev = nullptr;
    sum->next = nullptr;
    sum->head = nullptr;
    sum->tail = nullptr;

    if (left->head->multiplier == 0 && right->head->multiplier == 0) {
        sum->head = new node;
        sum->head->multiplier = 0;
        sum->tail = sum->head;
        for (int i = 0; i < 26; ++i) {
            sum->head->variables[i] = 0;
        }
        add_to_table(sum);
        return;
    }
    if (left->head->multiplier == 0) {
        node* tmp = right->head;
        while (tmp != nullptr) {
            if (!check_inclusion(tmp->multiplier, tmp->variables, sum)) {
                add_monomial(tmp->multiplier, tmp->variables, sum);
            }
            tmp = tmp->next;
        }
        add_to_table(sum);
        return;
    }
    if (right->head->multiplier == 0) {
        node* tmp = left->head;
        while (tmp != nullptr) {
            if (!check_inclusion(tmp->multiplier, tmp->variables, sum)) {
                add_monomial(tmp->multiplier, tmp->variables, sum);
            }
            tmp = tmp->next;
        }
        add_to_table(sum);
        return;
    }

    node* tmp = left->head;
    while (tmp != nullptr) {
        if (!check_inclusion(tmp->multiplier, tmp->variables, sum)) {
            add_monomial(tmp->multiplier, tmp->variables, sum);
        }
        tmp = tmp->next;
    }
    tmp = right->head;
    while (tmp != nullptr) {
        if (!check_inclusion(tmp->multiplier, tmp->variables, sum)) {
            add_monomial(tmp->multiplier, tmp->variables, sum);
        }
        tmp = tmp->next;
    }
    add_to_table(sum);
    return;
}

void MainWindow::on_plus_clicked()
{
    try {
        int left_num = ui->left->text().toInt();
        int right_num = ui->right->text().toInt();
        if (left_num > ui->tableWidget->rowCount()
                || right_num > ui->tableWidget->rowCount()) {
            throw -1;
        }
        node_base* left = find_in_base(left_num);
        node_base* right = find_in_base(right_num);
        plus(left, right);
    }  catch (int err) {
        QMessageBox msgBox(QMessageBox::Warning,
                           QString::fromUtf8("Error!"),
                           QString::fromUtf8("There is no such index in the table!"));
        msgBox.exec();
    }
}

void roots(node_base* ptr_base)
{
    node* ptr = ptr_base->head;
    std::vector <bool> vars(26, false);
    try {
        while (ptr != nullptr) {
            for (int i = 0; i < 26; ++i) {
                if (ptr->variables[i] > 0) {
                    vars[i] = true;
                }
            }
            ptr = ptr->next;
        }
        int var = -1;
        for (int i = 0; i < 26; ++i) {
            if (var == -1 && vars[i]) {
                var = i;
            } else if (vars[i]) {
                throw -1;
            }
        }
        if (var == -1){
            throw 1.0;
        }
        int min = 1e9;
        int mul;
        ptr = ptr_base->head;
        while (ptr != nullptr) {
            if (ptr->variables[var] < min) {
                min = ptr->variables[var];
                mul = abs(ptr->multiplier);
            }
            ptr = ptr->next;
        }
        std::vector <int> root;
        for (int i = -mul; i <= mul; ++i) {
            if (i == 0) continue;
            if (mul % i == 0) {
                ptr = ptr_base->head;
                int val = 0;
                while (ptr != nullptr) {
                    val += ptr->multiplier * std::pow(i, ptr->variables[var]);
                    ptr = ptr->next;
                }
                if (val == 0) {
                    root.push_back(i);
                }
            }
        }
        if (min > 0) {
            root.push_back(0);
        }
        QString res("");
        if (root.size() == 0) {
            res = "No integer roots!";
        } else {
            res = "Roots: ";
            for (auto elem : root) {
                res += QString::number(elem);
                res += ", ";
            }
            res = res.left(res.size() - 2);
        }
        QMessageBox msgBox;
        msgBox.setWindowTitle("Result");
        msgBox.setText(res);
        msgBox.exec();

    }  catch (int err) {
        QMessageBox msgBox(QMessageBox::Warning,
                           QString::fromUtf8("Error!"),
                           QString::fromUtf8("This polinome contains more than 1 variable!"));
        msgBox.exec();
    } catch (double err) {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Result");
        msgBox.setText("No integer roots!");
        msgBox.exec();
    }
}

void MainWindow::on_roots_clicked()
{
    try {
        int num = ui->number->text().toInt();
        if (num > ui->tableWidget->rowCount()) {
            throw -1;
        }
        node_base* ptr_base = find_in_base(num);
        roots(ptr_base);
    }  catch (int err) {
        QMessageBox msgBox(QMessageBox::Warning,
                           QString::fromUtf8("Error!"),
                           QString::fromUtf8("There is no such index in the table!"));
        msgBox.exec();
    }
}

void MainWindow::on_del_clicked()
{
    try {
        int num = ui->del_num->text().toInt();
        if (num > ui->tableWidget->rowCount()) {
            throw -1;
        }
        node_base* ptr_base = find_in_base(num);
        del_polynome(ptr_base);
        ui->tableWidget->removeRow(num-1);
    }  catch (int err) {
        QMessageBox msgBox(QMessageBox::Warning,
                           QString::fromUtf8("Error!"),
                           QString::fromUtf8("There is no such index in the table!"));
        msgBox.exec();
    }

}

void MainWindow::mul(node_base* left, node_base* right, int mode)
{
    node_base* mult = new node_base;
    mult->prev = nullptr;
    mult->next = nullptr;
    mult->head = nullptr;
    mult->tail = nullptr;
    if (left->head->multiplier == 0 || right->head->multiplier == 0) {
        mult->head = new node;
        mult->head->multiplier = 0;
        mult->head->next = nullptr;
        mult->head->prev = nullptr;
        mult->tail = mult->head;
        for (int i = 0; i < 26; ++i) {
            mult->head->variables[i] = 0;
        }
        add_to_table(mult, mode);
        return;
    }
    node* ptr1 = left->head;
    while (ptr1 != nullptr) {
        node* ptr2 = right->head;
        while (ptr2 != nullptr) {
            int multiplier = ptr1->multiplier * ptr2->multiplier;
            std::vector <int> vars(26, 0);
            for (int i = 0; i < 26; ++i) {
                vars[i] += ptr1->variables[i] + ptr2->variables[i];
            }
            if (!check_inclusion(multiplier, vars, mult)) {
                add_monomial(multiplier, vars, mult);
            }
            ptr2 = ptr2->next;
        }
        ptr1 = ptr1->next;
    }
    add_to_table(mult, mode);
}

void MainWindow::on_mul_clicked()
{
    try {
        int left_num = ui->left->text().toInt();
        int right_num = ui->right->text().toInt();
        if (left_num > ui->tableWidget->rowCount()
                || right_num > ui->tableWidget->rowCount()) {
            throw -1;
        }
        node_base* left = find_in_base(left_num);
        node_base* right = find_in_base(right_num);
        mul(left, right);

    }  catch (int err) {
        QMessageBox msgBox(QMessageBox::Warning,
                           QString::fromUtf8("Error!"),
                           QString::fromUtf8("There is no such index in the table!"));
        msgBox.exec();
    }
}

void MainWindow::on_pushButton_clicked()
{
    std::fstream out("output.txt", std::ios::out);
    for (int i = 0; i < ui->tableWidget->rowCount(); ++i) {
        out << ui->tableWidget->item(i, 0)->text().toStdString() << '\n';
    }
    out.close();
}

void MainWindow::on_pushButton_2_clicked()
{
    std::fstream in("output.txt", std::ios::in);
    while (!in.eof()) {
        std::string tmp;
        std::getline(in, tmp);
        if (tmp == "") break;
        QString input(QString::fromStdString(tmp));
        if (!state_machine(input, 1)) continue;
        add_from_textbox(input);
        ui->tableWidget->insertRow(ui->tableWidget->rowCount());
        ui->tableWidget->setItem(ui->tableWidget->rowCount()-1, 0,
                                 new QTableWidgetItem(construct_polynome(tail)));
    }
    in.close();
}

void MainWindow::value(node_base* ptr_base)
{
    node* ptr = ptr_base->head;
    std::vector <bool> vars(26, false);
    while (ptr != nullptr) {
        for (int i = 0; i < 26; ++i) {
            if (ptr->variables[i] > 0) {
                vars[i] = true;
            }
        }
        ptr = ptr->next;
    }
    std::vector <int> values_of_vars(26, -2e9);
    for (int i = 0; i < 26; ++i) {
        if (vars[i]) {
            QString res("Input value of variable ");
            QChar var(97 + i);
            res += var;
            res += ": ";
            values_of_vars[i] = QInputDialog::getInt(this, QString::fromUtf8("Ввод значения"), res);
        }
    }
    ptr = ptr_base->head;
    int val_of_func = 0;
    while (ptr != nullptr) {
        int tmp_val = ptr->multiplier;
        for (int i = 0; i < 26; ++i) {
            if (vars[i] && ptr->variables[i] > 0) {
                tmp_val *= std::pow(values_of_vars[i], ptr->variables[i]);
            }
        }
        val_of_func += tmp_val;
        ptr = ptr->next;
    }
    QMessageBox msgBox;
    msgBox.setWindowTitle("Result");
    QString res("Value of function is ");
    res += QString::number(val_of_func);
    msgBox.setText(res);
    msgBox.exec();
}

void MainWindow::on_der_2_clicked()
{
    try {
        int num = ui->del_num->text().toInt();
        if (num > ui->tableWidget->rowCount()) {
            throw -1;
        }
        node_base* ptr_base = find_in_base(num);
        value(ptr_base);

    }  catch (int err) {
        QMessageBox msgBox(QMessageBox::Warning,
                           QString::fromUtf8("Error!"),
                           QString::fromUtf8("There is no such index in the table!"));
        msgBox.exec();
    }
}

void MainWindow::minus(node_base* left, node_base* right, int mode)
{
    node_base* sum = new node_base;
    sum->prev = nullptr;
    sum->next = nullptr;
    sum->head = nullptr;
    sum->tail = nullptr;

    if (left->head->multiplier == 0 && right->head->multiplier == 0) {
        sum->head = new node;
        sum->head->multiplier = 0;
        sum->tail = sum->head;
        for (int i = 0; i < 26; ++i) {
            sum->head->variables[i] = 0;
        }
        add_to_table(sum, mode);
        return;
    }
    if (left->head->multiplier == 0) {
        node* tmp = right->head;
        while (tmp != nullptr) {
            if (!check_inclusion(-tmp->multiplier, tmp->variables, sum)) {
                add_monomial(-tmp->multiplier, tmp->variables, sum);
            }
            tmp = tmp->next;
        }
        add_to_table(sum, mode);
        return;
    }
    if (right->head->multiplier == 0) {
        node* tmp = left->head;
        while (tmp != nullptr) {
            if (!check_inclusion(tmp->multiplier, tmp->variables, sum)) {
                add_monomial(tmp->multiplier, tmp->variables, sum);
            }
            tmp = tmp->next;
        }
        add_to_table(sum, mode);
        return;
    }

    node* tmp = left->head;
    while (tmp != nullptr) {
        if (!check_inclusion(tmp->multiplier, tmp->variables, sum)) {
            add_monomial(tmp->multiplier, tmp->variables, sum);
        }
        tmp = tmp->next;
    }
    tmp = right->head;
    while (tmp != nullptr) {
        if (!check_inclusion(-tmp->multiplier, tmp->variables, sum)) {
            add_monomial(-tmp->multiplier, tmp->variables, sum);
        }
        tmp = tmp->next;
    }
    add_to_table(sum, mode);
    return;
}

void MainWindow::on_minus_clicked()
{
    try {
        int left_num = ui->left->text().toInt();
        int right_num = ui->right->text().toInt();
        if (left_num > ui->tableWidget->rowCount()
                || right_num > ui->tableWidget->rowCount()) {
            throw -1;
        }
        node_base* left = find_in_base(left_num);
        node_base* right = find_in_base(right_num);
        minus(left, right);
    }  catch (int err) {
        QMessageBox msgBox(QMessageBox::Warning,
                           QString::fromUtf8("Error!"),
                           QString::fromUtf8("There is no such index in the table!"));
        msgBox.exec();
    }
}

node* find_max_pow(node_base* ptr_base, int var) {
    node* ptr = ptr_base->head;
    int max = -1;
    node* ptr_max = nullptr;
    while (ptr != nullptr) {
        if (ptr->variables[var] > max) {
            max = ptr->variables[var];
            ptr_max = ptr;
        }
        ptr = ptr->next;
    }
    return ptr_max;
}

void MainWindow::division(node_base* left, node_base* right)
{
    try {
        if (right->head->multiplier == 0) throw 0;
        if (left->head->multiplier == 0) {
            QMessageBox msgBox;
            msgBox.setWindowTitle("Result");
            msgBox.setText("Quotient: 0\nRemainder: 0");
            msgBox.exec();
            return;
        }

        node* ptr = left->head;
        std::vector <bool> varsl(26, false);
        while (ptr != nullptr) {
            for (int i = 0; i < 26; ++i) {
                if (ptr->variables[i] > 0) {
                    varsl[i] = true;
                }
            }
            ptr = ptr->next;
        }

        ptr = right->head;
        std::vector <bool> varsr(26, false);
        while (ptr != nullptr) {
            for (int i = 0; i < 26; ++i) {
                if (ptr->variables[i] > 0) {
                    varsr[i] = true;
                }
            }
            ptr = ptr->next;
        }

        int cnt1 = 0;
        for (auto elem : varsl) {
            if (elem) ++cnt1;
        }
        if (cnt1 > 1) throw 1.0;

        int cnt2 = 0;
        for (auto elem : varsr) {
            if (elem) ++cnt2;
        }
        if (cnt2 > 1) throw 1.0;

        if (varsl != varsr && (cnt1 != 0 && cnt2 != 0)) throw 1.0;

        node_base* tmpp = new node_base;
        tmpp->head = nullptr;
        tmpp->tail = nullptr;
        tmpp->prev = nullptr;
        tmpp->next = nullptr;

        ptr = left->head;
        while (ptr != nullptr) {
            add_monomial(ptr->multiplier, ptr->variables, tmpp);
            ptr = ptr->next;
        }

        node_base* quotient = new node_base;
        quotient->head = nullptr;
        quotient->tail = nullptr;
        quotient->prev = nullptr;
        quotient->next = nullptr;

        int var = -1;
        for (int i = 0; i < 26; ++i) {
            if (varsr[i]) var = i;
        }

        while (tmpp->head != nullptr && tmpp->head->multiplier != 0) {
            node* max = find_max_pow(tmpp, var);
            if (max->multiplier / (double)right->head->multiplier
                    != max->multiplier / right->head->multiplier) {
                if (quotient->head == nullptr) {
                    add_monomial(0,
                                 {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                                 quotient);
                    node_base* t = new node_base;
                    t->head = nullptr;
                    t->tail = nullptr;
                    t->prev = nullptr;
                    t->next = nullptr;
                    ptr = left->head;
                    while (ptr != nullptr) {
                        add_monomial(ptr->multiplier, ptr->variables, t);
                        ptr = ptr->next;
                    }
                    add_polynome(t);
                }
                break;
            }
            node_base* cur_mon = new node_base;
            cur_mon->head = nullptr;
            cur_mon->tail = nullptr;
            cur_mon->prev = nullptr;
            cur_mon->next = nullptr;

            max->variables[var] -= right->head->variables[var];
            add_monomial(max->multiplier / right->head->multiplier, max->variables, cur_mon);
            add_monomial(max->multiplier / right->head->multiplier, max->variables, quotient);
            max->variables[var] += right->head->variables[var];
            mul(right, cur_mon, 0);
            minus(tmpp, tail, 0);
            delete tmpp;
            tmpp = tail;
            del_polynome(tail->prev);
            //delete cur_mon;
        }

        QMessageBox msgBox;
        msgBox.setWindowTitle("Result");
        QString res("Quotient: ");
        res += construct_polynome(quotient);
        res += "\nRemainder: ";
        res += construct_polynome(tail);
        msgBox.setText(res);
        msgBox.exec();

        del_polynome(tail);
        delete quotient;
        return;

    } catch (int zero) {
        QMessageBox msgBox(QMessageBox::Warning,
                           QString::fromUtf8("Error!"),
                           QString::fromUtf8("Division by zero!"));
        msgBox.exec();
    } catch (double err) {
        QMessageBox msgBox(QMessageBox::Warning,
                           QString::fromUtf8("Error!"),
                           QString::fromUtf8("There is more than 1 variable!"));
        msgBox.exec();
    }
}

void MainWindow::on_div_clicked()
{
    try {
        int left_num = ui->left->text().toInt();
        int right_num = ui->right->text().toInt();
        if (left_num > ui->tableWidget->rowCount()
                || right_num > ui->tableWidget->rowCount()) {
            throw -1;
        }
        node_base* left = find_in_base(left_num);
        node_base* right = find_in_base(right_num);
        division(left, right);
    }  catch (int err) {
        QMessageBox msgBox(QMessageBox::Warning,
                           QString::fromUtf8("Error!"),
                           QString::fromUtf8("There is no such index in the table!"));
        msgBox.exec();
    }
}
