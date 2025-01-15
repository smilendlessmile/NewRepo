#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <regex>
#include <sstream>
#include <queue>
#include <functional>
#include <regex>
#include <filesystem>
#include<cstdio>
#include <Windows.h>

using namespace std;

// ��������
void CreateTable(string sql);//������
void InsertInto(string sql);//����
void AlterTable(string sql);//�޸ı�
void DeleteFrom(string sql);//ɾ��
void Updata(string sql);//����
void DropTable(string sql);//ɾ����
string ShowTable(string sql);//��ʾ��

HWND hEditInput;   // SQL �������
HWND hEditOutput;  // ����������

string ExecuteSQL(const wstring& sql) {
    string sqlStr(sql.begin(), sql.end());
    string result; // ���ڲ���ִ�н��

    if (sqlStr.find("CREATE TABLE") != string::npos) {
        CreateTable(sqlStr);
        result = "Table created successfully."; // �趨����ַ���
    }
    else if (sqlStr.find("INSERT INTO") != string::npos) {
        InsertInto(sqlStr);
        result = "Row inserted successfully."; // �趨����ַ���
    }
    else if (sqlStr.find("ALTER TABLE") != string::npos) {
        AlterTable(sqlStr);
        result = "Table altered successfully."; // �趨����ַ���
    }
    else if (sqlStr.find("DELETE FROM") != string::npos) {
        DeleteFrom(sqlStr);
        result = "Row deleted successfully."; // �趨����ַ���
    }
    else if (sqlStr.find("UPDATE") != string::npos) {
        Updata(sqlStr);
        result = "Table updated successfully."; // �趨����ַ���
    }
    else if (sqlStr.find("DROP TABLE") != string::npos) {
        DropTable(sqlStr);
        result = "Table dropped successfully."; // �趨����ַ���
    }
    else if (sqlStr.find("SELECT * FROM") != string::npos) {
        result = ShowTable(sqlStr); // ֱ�ӻ�ȡ��ʾ����
    }
    else {
        result = "û��ƥ��� SQL ��䡣"; // �趨�������ַ���
    }

    return result;
}


LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_CREATE: {
        // ���� SQL �����
        hEditInput = CreateWindowEx(
            WS_EX_CLIENTEDGE, L"EDIT", L"",
            WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE,
            10, 10, 560, 50, hwnd, NULL, GetModuleHandle(NULL), NULL
        );


        // ����ִ�а�ť
        CreateWindow(
            L"BUTTON", L"Execute",
            WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            10, 70, 100, 30, hwnd, (HMENU)1, GetModuleHandle(NULL), NULL
        );

		// ���������
        hEditOutput = CreateWindowEx(
            WS_EX_CLIENTEDGE, L"EDIT", L"",
            WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_READONLY,
            10, 110, 560, 500, hwnd, NULL, GetModuleHandle(NULL), NULL
        );

        break;
    }

	// ִ���߼�
    case WM_COMMAND: {
        if (LOWORD(wParam) == 1) { // ��������ִ�а�ť
            wchar_t sql[256]; // ʹ�ÿ��ַ�����
            GetWindowText(hEditInput, sql, lstrlenW(sql) + 1); // ��ȡ����� SQL ���
            string result = ExecuteSQL(wstring(sql, sql + wcslen(sql))); // ��ȡִ�н���ַ���

            // ��ʾִ�н��
            SetWindowText(hEditOutput, wstring(result.begin(), result.end()).c_str()); // ���������
        }
        break;
    }

	 // ��������
    case WM_DESTROY: {
        PostQuitMessage(0);
        break;
    }
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}


// ���ݱ���еĽṹ��
struct Column {
    string name;            // ����
    string type;            // �е���������
    bool isPrimaryKey;      // �Ƿ�������
    bool isNullable;        // �Ƿ����Ϊ��
    bool isUnique;          // �Ƿ�Ψһ
};

// ���ݱ�Ľṹ��
struct Table {
    string name;            // ����
    vector<Column> columns; // �еļ���
};

// �����ֵ䣬�Ա�����Ϊ�����洢���Ԫ��Ϣ
map<string, Table> dataDictionary;

//���ݿ��и���洢������ ���� ���� ���� ��������
map<string, map<string, map<string, string>>> TableData;

//�洢��ѯ���
queue<string> out;

// ����һ��ӳ�䣬��������ӳ�䵽��Ӧ�ĺ�������
map<string, function<bool(int, int)>> operators;

// ����������ĳһ�е���������
string findColumnType(string tableName, string columnName) {
    // ����dataDictionary
    for (const auto& kvp : dataDictionary) {
        const Table& table = kvp.second;
        cout << "Table Name: " << table.name << endl;
        for (const Column& column : table.columns) {
            cout << "  Data Type: " << column.type << endl;
            if (columnName == column.name) {
                return column.type;
            }
        }
    }
    return "Column not found"; // ���߷����ʵ��Ĵ���ֵ
}

void InitOperators() {

    operators["<"] = less<int>();
    operators[">"] = greater<int>();
    operators["<="] = less_equal<int>();
    operators[">="] = greater_equal<int>();
    operators["="] = equal_to<int>();
    operators["!="] = not_equal_to<int>();
}

void PrintOutTableData(HWND hEditOutput) {
    string output;

    // ��������
    for (const auto& tableEntry : TableData) {
        const string& tableName = tableEntry.first;
        output += "����: " + tableName + "\r\n";

        // ��������
        for (const auto& columnEntry : tableEntry.second) {
            const string& columnName = columnEntry.first;
            output += "  ����: " + columnName + "\r\n";

            // ������������������
            for (const auto& dataEntry : columnEntry.second) {
                const string& key = dataEntry.first;
                const string& value = dataEntry.second;

                if (key == "����") {
                    output += "    ����: " + value + "\r\n";
                }
                else {
                    output += "    ��������: " + key + " = " + value + "\r\n";
                }
            }
        }
        output += "\r\n"; // ÿ����֮���һ������
    }

    // �������ʾ���������
    SetWindowText(hEditOutput, wstring(output.begin(), output.end()).c_str());
}



void PrintOutDataDictionary(HWND hEditOutput) {
    string output;

    for (const auto& entry : dataDictionary) {
        const string& tableName = entry.first;
        const Table& tableInfo = entry.second;

        output += "TableName: " + tableName + "\r\n";

        // �������Ϣ
        for (const Column& column : tableInfo.columns) {
            output += "ColumnName: " + column.name + "\r\n";
            output += "DateType: " + column.type + "\r\n";
            output += "ISKey: " + string(column.isPrimaryKey ? "Yes" : "No") + "\r\n";
            output += "ISNULL: " + string(column.isNullable ? "Yes" : "No") + "\r\n";
            output += "ISONLY: " + string(column.isUnique ? "Yes" : "No") + "\r\n";
            output += "\r\n"; // ÿ������Ϣ֮���һ������
        }
    }

    // �������ʾ���������
    SetWindowText(hEditOutput, wstring(output.begin(), output.end()).c_str());
}


//д�����ݱ�
void WriteTable(string tableName) {
    //д��map ���� ���� ��������
    map<string, map<string, string>> write;

    PrintOutDataDictionary(hEditOutput);

    // ����TableData
    for (const auto& tableEntry : TableData) {
        if (tableEntry.first == tableName) {
            const auto& table = tableEntry.second;
            // �������е���
            for (const auto& columnEntry : table) {
                const auto& column = columnEntry.second;
                const string& columnName = columnEntry.first;

                // �������е���������������
                for (const auto& primaryKeyEntry : column) {
                    const string& key = primaryKeyEntry.first;
                    const string& data = primaryKeyEntry.second;
                    write[key][columnName] = data;
                }
            }
        }
    }

    // ���ļ��Ը���д������
    ofstream file(tableName + ".txt");

    if (!file.is_open()) {
        cerr << "�޷����ļ�" << endl;
    }
    // ʹ�÷�Χѭ������write map
    for (const auto& entry : write) {
        string key = entry.first;

        // ����һ����ʱ����������ԭ�����е�Ԫ��
        queue<string> tempQueue = out;
        string line;
        // ����ԭ����
        while (!tempQueue.empty()) {
            string frontElement = tempQueue.front(); // ��ȡ��ͷԪ��

            if (!write[key][frontElement].empty()) {
                line += write[key][frontElement] + " ";
                cout << frontElement << " " << write[key][frontElement] << endl;
            }
            tempQueue.pop();                        // ������ͷԪ��
        }

        file << line << endl;
    }

    // �ر��ļ�
    file.close();

}

void ReadTable(string tableName) {
    ifstream inputFile(tableName + ".txt");
    if (!inputFile) {
        string errorOutput = "�޷����ļ�: " + tableName + ".txt\r\n";
        SetWindowText(hEditOutput, wstring(errorOutput.begin(), errorOutput.end()).c_str());
        return;
    }

    string line;
    while (getline(inputFile, line)) {
        istringstream iss(line);
        vector<string> words;
        string word;

        // ʹ�ÿո�ָ���
        while (iss >> word) {
            words.push_back(word);
        }

        // �������ֵ��в��ұ��
        auto tableIt = dataDictionary.find(tableName);
        if (tableIt != dataDictionary.end()) {
            const Table& table = tableIt->second;

            // �����һ��Ϊ����
            const string& primaryKey = words[0];
            for (size_t i = 0; i < table.columns.size() && i < words.size(); i++) {
                const Column& column = table.columns[i];
                TableData[tableName][column.name][primaryKey] = words[i];
            }
        }
    }
    inputFile.close();
}



// ���������÷�ʽ����һ���ַ������Ӹ��ַ�����ɾ�����еĵ�����
string removeSingleQuotes(string str) {
    size_t pos = str.find('\'');
    while (pos != string::npos) {
        str.erase(pos, 1); // ���ַ�����ɾ��һ���ַ�
        pos = str.find('\'', pos); // ����������һ��������
    }
    return str;
}

//intת��Ϊ�ַ���
string IntToString(int num) {
    string str = to_string(num);
    return str;
}

//�ַ���ת��Ϊint
int StringToInt(string str) {
    int num = stoi(str);
    return num;
}


//����ָ�����ֵ�.txt�ļ�
void CreateFile(string filename) {

    // ����һ������ļ�������
    ofstream outFile(filename + ".txt");

    if (outFile.is_open()) {
        // �ر��ļ�
        outFile.close();
    }
}

//�ж����
bool Judge(string str) {
    if (str == "true") {
        return true;
    }
    else {
        return false;
    }
}

//�ж����
string JudgeBack(bool judge) {
    if (judge) return "true";
    else return "false";
}

//��ȡ��������Ϣ
string extractContentInParentheses(const string& input) {
    size_t startPos = input.find('(');
    size_t endPos = input.find(')', startPos);

    if (startPos != string::npos && endPos != string::npos) {
        return input.substr(startPos + 1, endPos - startPos - 1);
    }
    else {
        return ""; // ���δ�ҵ���������
    }
}

//���������ֵ��еı����
void AddTable(string str) {
    Table newTable;
    // ���±���ӵ������ֵ���
    dataDictionary[str] = newTable;
}

//���������ֵ��е������
void DealColumnString(string TableName, string str) {
    Column newcolumn;
    istringstream ss(str);
    string name;
    string type;
    string primaryKey;
    string nullable;
    string unique;
    ss >> name >> type >> primaryKey >> nullable >> unique;
    out.push(name);
    newcolumn.name = name;
    newcolumn.type = type;
    newcolumn.isPrimaryKey = Judge(primaryKey);
    newcolumn.isNullable = Judge(nullable);
    newcolumn.isUnique = Judge(unique);
    dataDictionary[TableName].columns.push_back(newcolumn);
}

//��ȡ�����ֵ�
void ReadDataDictionary(string line) {
    vector<string> result;
    istringstream read(line);
    string token;
    while (getline(read, token, '/')) {
        result.push_back(token);
    }
    bool first = true;
    string TableName;
    for (const string& str : result) {
        if (first) {
            AddTable(str);
            TableName = str;
            first = false;
        }
        else {
            DealColumnString(TableName, str);
        }
    }
}

// �����ָ��������Ϊ����
void setColumnAsPrimaryKey(const string& tableName, const string& columnName) {
    // ���ȼ����Ƿ�����������ֵ���
    if (dataDictionary.find(tableName) != dataDictionary.end()) {
        Table& table = dataDictionary[tableName];
        for (Column& column : table.columns) {
            if (column.name == columnName) {
                column.isPrimaryKey = true;
                column.isNullable = false;
                column.isUnique = true;
                return;
            }
        }
    }
}

// ������
string FindPrimaryKey(const string& tableName) {
    // ���ȼ����Ƿ�����������ֵ���
    if (dataDictionary.find(tableName) != dataDictionary.end()) {
        Table& table = dataDictionary[tableName];
        for (Column& column : table.columns) {
            if (column.isPrimaryKey) {
                return column.name;
            }
        }
    }
    return "not found";
}

void fillNULL(string tableNameToModify, string columnNameToChange) {
    for (const auto& tableEntry : TableData) {
        if (tableNameToModify == tableEntry.first) {
            for (const auto& columnEntry : tableEntry.second) {
                string columnName = columnEntry.first;
                if (columnNameToChange == columnName) {
                    for (const auto& rowEntry : columnEntry.second) {
                        string primaryKey = rowEntry.first;
                        TableData[tableNameToModify][columnName][primaryKey] = "NULL";
                    }
                }
            }
        }
    }
    PrintOutTableData(hEditOutput);
}

//create table�������ݿ���������
void CreateColumn(string tableName, string str) {
    if (str.find("PRIMARY KEY") != string::npos) {
        string primaryKey = extractContentInParentheses(str);
        setColumnAsPrimaryKey(tableName, primaryKey);
    }
    else {
        Column newcolumn;
        istringstream ss(str);
        string name;
        string type;
        ss >> name >> type;
        newcolumn.name = name;
        newcolumn.type = type;
        newcolumn.isPrimaryKey = false;
        newcolumn.isNullable = true;
        newcolumn.isUnique = false;
        dataDictionary[tableName].columns.push_back(newcolumn);
    }
}

//create table�������ݿ��
void CreateTable(string sql) {
    // ����һ��������ʽģʽ��ƥ��CREATE TABLE���
    regex pattern(R"((CREATE TABLE) ([A-Za-z_][A-Za-z0-9_]*) \(([^;]*)\);)");

    // �������ַ���������ƥ����
    smatch match;
    string createTable;
    string tableName;
    string columnInfo;
    if (regex_search(sql, match, pattern)) {
        // match[0] ��������ƥ����ַ���
        // match[1] ���� "CREATE TABLE"
        // match[2] ��������
        // match[3] ��������Ϣ
        createTable = match[1];
        tableName = match[2];
        columnInfo = match[3];
    }
    else {
        cout << "δ�ܳɹ�ƥ�䣡" << endl;
    }
	AddTable(tableName);//��������ӵ������ֵ�
	vector<string> result;
    istringstream read(columnInfo);
    string token;
	// �Զ��ŷָ�����
    while (getline(read, token, ',')) {
        result.push_back(token);
    }
	// ��������ӵ������ֵ�
    for (const string& str : result) {
        CreateColumn(tableName, str);
    }
    CreateFile(tableName);
}


//���ļ���ȡ�����ֵ�
void InitDataDictionary() {
    // ���ļ��Խ��ж�ȡ
    ifstream inputFile("dataDictionary.txt");

    if (!inputFile.is_open()) {
        // ����ļ������ڣ�����һ�����ļ�
        ofstream outputFile("dataDictionary.txt");
        if (outputFile.is_open()) {
            cout << "���ݿⲻ���ڣ��Ѵ������ݿ�" << endl;
            // ����������д�����ļ�������
            outputFile << "����һ���´��������ݿ⡣\n";
            outputFile.close();
        }
        else {
            cerr << "�޷��������ݿ�'" << endl;
        }
    }
    else {
        string line;
        while (getline(inputFile, line)) {
            // ���������ֵ��ļ��е�����
            ReadDataDictionary(line);
        }
        inputFile.close();
    }
}


//д������
void WriteDataDictionary() {
    // ���ļ��������ļ�ģʽΪ����͸���
    ofstream outputFile("DataDictionary.txt", ios::out | ios::trunc);
    if (!outputFile.is_open()) {
        cerr << "�޷����ļ� " << "DataDictionary.txt" << endl;
    }

    // ����ÿ�������Ϣ
    for (const auto& entry : dataDictionary) {
        const string& tableName = entry.first;
        const Table& tableInfo = entry.second;
        string write;
        write = tableName;

        // ��������Ϣ
        for (const Column& column : tableInfo.columns) {
            write += "/" + column.name + " " + column.type + " " + (column.isPrimaryKey ? "true" : "false") + " " + (column.isNullable ? "true" : "false") + " " + (column.isUnique ? "true" : "false");

        }
        outputFile << write << endl;
    }
    // �ر��ļ�
    outputFile.close();
}

// �������б��INSERT INTO���
void InsertIntoWithColumns(string sql) {
    // ����һ��������ʽģʽ��ƥ��INSERT INTO���
    regex pattern(R"((INSERT INTO) ([A-Za-z_][A-Za-z0-9_]*) \(([^)]*)\) VALUES \(([^)]*)\);)");

    string insertInto;
    string tableName;
    string columnNames;
    string values;

    // �������ַ���������ƥ����
    smatch match;
    if (regex_search(sql, match, pattern)) {
        // match[0] ��������ƥ����ַ���
        // match[1] ���� "INSERT INTO"
        // match[2] ��������
        // match[3] ���������б�
        // match[4] ����ֵ�б�
        insertInto = match[1];
        tableName = match[2];
        columnNames = match[3];
        values = removeSingleQuotes(match[4]);
        cout << tableName << endl << columnNames << endl << values << endl;
    }
    else {
        cout << "No match found." << endl;
    }

    vector<string> valueresult;
    istringstream valueread(values);
    string valuetoken;

    while (getline(valueread, valuetoken, ',')) {
        valueresult.push_back(valuetoken);
    }

    vector<string> columnresult;
    istringstream columnread(columnNames);
    string columntoken;

    while (getline(columnread, columntoken, ',')) {
        columnresult.push_back(columntoken);
    }

    // ���ļ��������ļ�ģʽΪ�����׷��
    ofstream outputFile(tableName + ".txt", ios::out | ios::app);

    if (!outputFile.is_open()) {
        cerr << "�޷����ļ� " << tableName + ".txt" << endl;
    }

    for (size_t i = 0; i < columnresult.size(); i++) {
        string ColumnName = removeSingleQuotes(columnresult[i]);
        string Value = removeSingleQuotes(valueresult[i]);
        //�������ֵ��в��Ҹñ��Ƿ��и���
        auto tableIt = dataDictionary.find(tableName);
        if (tableIt != dataDictionary.end()) {
            const Table& table = tableIt->second;
            for (const Column& column : table.columns) {
                if (column.name == ColumnName) {
                    // �ҵ�����

                    TableData[tableName][ColumnName][FindPrimaryKey(tableName)] = Value;

                }
            }
        }
    }

    string line;

    for (const string& str : columnresult) {
        string Str = TableData[tableName][str][FindPrimaryKey(tableName)];
        if (Str.empty()) {
            Str = "NULL";
            line += Str + " ";
        }
        else {
            line += Str + " ";
        }
    }

    // ����д���ļ�����ν���������ǰ�����ݣ�
    outputFile << line << endl;

    // �ر��ļ�
    outputFile.close();
}

// û�������б��INSERT INTO���
void InsertIntoWithoutColumns(string sql) {

    // ����һ��������ʽģʽ��ƥ��INSERT INTO���
    regex pattern(R"((INSERT INTO) ([A-Za-z_][A-Za-z0-9_]*) (?:\([^)]*\) )?VALUES \(([^)]*)\);)");

    string insertInto;
    string tableName;
    string columnNames;
    string values;

    // �������ַ���������ƥ����
    smatch match;
    if (regex_search(sql, match, pattern)) {
        // match[0] ��������ƥ����ַ���
        // match[1] ���� "INSERT INTO"
        // match[2] ��������
        // match[3] ����ֵ�б�
        insertInto = match[1];
        tableName = match[2];
        values = match[3];
    }
    else {
        cout << "No match found." << endl;
    }

    vector<string> result;
    istringstream read(values);
    string token;

    while (getline(read, token, ',')) {
        result.push_back(token);
    }

    // ���ļ��������ļ�ģʽΪ�����׷��
    ofstream outputFile(tableName + ".txt", ios::out | ios::app);

    if (!outputFile.is_open()) {
        cerr << "�޷����ļ� " << tableName + ".txt" << endl;
    }

    string line;

    for (const string& str : result) {
        string Str = removeSingleQuotes(str);
        line += Str + " ";
    }

    // ����д���ļ�����ν���������ǰ�����ݣ�
    outputFile << line << endl;

    // �ر��ļ�
    outputFile.close();

}


//InsertIntoΪ��ϵ�����Ԫ��
void InsertInto(string sql) {
    // ����һ��������ʽģʽ��ƥ��INSERT INTO���
    regex patternWithColumns(R"((INSERT INTO) ([A-Za-z_][A-Za-z0-9_]*) \(([^)]*)\) VALUES \(([^)]*)\);)");
    regex patternWithoutColumns(R"((INSERT INTO) ([A-Za-z_][A-Za-z0-9_]*) VALUES \(([^)]*)\);)");

    // �������ַ���������ƥ����
    smatch matchWithColumns;
    smatch matchWithoutColumns;

    if (regex_search(sql, matchWithColumns, patternWithColumns)) {
        // ƥ�䵽�������б��INSERT INTO���
        InsertIntoWithColumns(sql);
    }
    else if (regex_search(sql, matchWithoutColumns, patternWithoutColumns)) {
        // ƥ�䵽û�������б��INSERT INTO���
        InsertIntoWithoutColumns(sql);
    }
    else {
        cout << "No match found." << endl;
    }
}

void Create(string tableName, string str) {
    Column newcolumn;
    istringstream ss(str);
    string name;
    string type;
    ss >> name >> type;
    newcolumn.name = name;
    newcolumn.type = type;
    newcolumn.isPrimaryKey = false;
    newcolumn.isNullable = true;
    newcolumn.isUnique = false;
    dataDictionary[tableName].columns.push_back(newcolumn);
    out.push(name);
    cout << name << endl;
    fillNULL(tableName, name);
}

//AlterADD�������
void AlterADD(string sql) {

    // ����һ��������ʽģʽ��ƥ��ALTER TABLE���
    regex pattern(R"((ALTER TABLE) ([A-Za-z_][A-Za-z0-9_]*) ADD \(([^;]*)\);)");

    // �������ַ���������ƥ����
    smatch match;
    string alterTable;
    string tableName;
    string columnInfo;

    if (regex_search(sql, match, pattern)) {
        // match[0] ��������ƥ����ַ���
        // match[1] ���� "ALTER TABLE"
        // match[2] ��������
        // match[3] ����Ҫ��ӵ�����Ϣ
        alterTable = match[1];
        tableName = match[2];
        columnInfo = match[3];
    }
    else {
        cout << "No match found." << endl;
    }

    ReadTable(tableName);

    Create(tableName, columnInfo);

    WriteTable(tableName);
}



// �����������ݱ���ɾ��ָ����
void deleteColumnFromTable(string tableName, string columnName) {
    if (dataDictionary.find(tableName) != dataDictionary.end()) {
        Table& table = dataDictionary[tableName];

        auto it = find_if(table.columns.begin(), table.columns.end(),
            [columnName](const Column& col) {
                return col.name == columnName;
            });

        if (it != table.columns.end()) {
            table.columns.erase(it);
            cout << "Column '" << columnName << "' deleted from table '" << tableName << "'." << endl;
        }
        else {
            cout << "Column '" << columnName << "' not found in table '" << tableName << "'." << endl;
        }
    }
    else {
        cout << "Table '" << tableName << "' not found." << endl;
    }
}




//ɾ����Ԫ��
void DeleteColumn(string tableName, string columnName) {
    ReadTable(tableName);

    //��TableData��ɾ��
    auto tableIt = TableData.find(tableName);
    if (tableIt != TableData.end()) {
        auto columnIt = tableIt->second.find(columnName);
        if (columnIt != tableIt->second.end()) {
            tableIt->second.erase(columnIt);
            cout << "������ɾ��" << endl;
        }
    }

    //�������ֵ���ɾ��
    deleteColumnFromTable(tableName, columnName);

    //����д���ļ�
    WriteTable(tableName);
}

//AlterDROPɾ������
void AlterDROP(string sql) {

    // ����һ��������ʽģʽ��ƥ��ALTER TABLE����е�DROP����
    regex pattern(R"((ALTER TABLE) ([A-Za-z_][A-Za-z0-9_]*) (DROP) ([A-Za-z_][A-Za-z0-9_]*);)");

    // �������ַ���������ƥ����
    smatch match;
    string alterTable;
    string tableName;
    string operation;
    string columnName;

    if (regex_search(sql, match, pattern)) {
        // match[0] ��������ƥ����ַ���
        // match[1] ���� "ALTER TABLE"
        // match[2] ��������
        // match[3] ���� "DROP"
        // match[4] ����Ҫɾ��������
        alterTable = match[1];
        tableName = match[2];
        operation = match[3];
        columnName = match[4];
    }
    else {
        cout << "No match found." << endl;
    }
    if (FindPrimaryKey(tableName) != columnName) {
        DeleteColumn(tableName, columnName);
    }
    else {
        cout << "��������ɾ����" << endl;
    }
}

void AlterTable(string sql) {
    if (sql.find("ADD") != string::npos) {
        AlterADD(sql);
    }
    else if (sql.find("DROP") != string::npos) {
        AlterDROP(sql);
    }
}

// ����Ƕ�׵�map����ӡ����
void TraverseNestedMap() {
    for (const auto& entry : TableData) {
        std::cout << "Outer Key: " << entry.first << std::endl;

        const std::map<std::string, std::map<std::string, std::string>>& innerMap = entry.second;

        for (const auto& innerEntry : innerMap) {
            std::cout << "  Inner Key: " << innerEntry.first << std::endl;

            const std::map<std::string, std::string>& subInnerMap = innerEntry.second;

            for (const auto& subInnerEntry : subInnerMap) {
                std::cout << "    Sub Inner Key: " << subInnerEntry.first << " - Value: " << subInnerEntry.second << std::endl;
            }
        }
    }
}

// ɾ���ض��еĺ���
void DeleteTableRow(string tableName, string primaryKey) {
    TraverseNestedMap();
    // ����TableData�е�ÿ�����
    for (auto& table : TableData) {
        // ������ǰ����е�ÿһ��
        for (auto& column : table.second) {
            // ��鵱ǰ�����Ƿ����ָ��������
            auto primaryKeyIt = column.second.find(primaryKey);
            if (primaryKeyIt != column.second.end()) {
                // ����ҵ�������������ɾ����
                column.second.erase(primaryKey);

                // ����ɹ�ɾ����������Ϣ����������к������ı�ʶ
                cout << "�ɹ�ɾ������ " << table.first << " - " << column.first << " - " << primaryKey << endl;
            }
        }
    }
    TraverseNestedMap();
}

void DeleteFindWhereDouble(string tableName, string condition) {

    // ����һ��������ʽģʽ��ƥ��AND��OR������
    regex pattern0(R"(([^ ]+) (AND|OR) ([^ ]+))");
    string firstString;
    string operation;
    string secondString;

    // �������ַ���������ƥ����
    smatch match0;
    if (std::regex_search(condition, match0, pattern0)) {
        // match[0] ��������ƥ����ַ���
        // match[1] ������һ���ַ���
        // match[2] ����AND��OR������
        // match[3] �����ڶ����ַ���
        firstString = match0[1];
        operation = match0[2];
        secondString = match0[3];
    }
    else {
        cout << "No match found." << endl;
    }

    // ���������ַ���
    string op1, op2;
    regex pattern(R"((\w+)\s*(=|>|<|>=|<=)\s*['"]?(\w+)['"]?)");
    smatch match1, match2;
    string leftOperand1, leftOperand2;
    string rightOperand1, rightOperand2;
    queue<string> keyToDelete1, keyToDelete2;
    if (regex_search(firstString, match1, pattern)) {
        leftOperand1 = match1[1];
        op1 = match1[2];
        rightOperand1 = match1[3];
        // ����TableData
        for (const auto& tableEntry : TableData) {
            if (tableEntry.first == tableName) {
                const auto& table = tableEntry.second;
                // �������е���
                for (const auto& columnEntry : table) {
                    const auto& column = columnEntry.second;
                    const string& columnName = columnEntry.first;
                    if (columnName == leftOperand1) {
                        // �������е���������������
                        for (const auto& primaryKeyEntry : column) {
                            const string& key = primaryKeyEntry.first;
                            const string& data = primaryKeyEntry.second;
                            if (findColumnType(tableName, leftOperand1) == "INT") {

                                // ���ַ���ת��Ϊ����
                                int leftValue = stoi(data);
                                int rightValue = stoi(rightOperand1);

                                // ʹ������ӳ��ִ�бȽ�
                                bool result = operators[op1](leftValue, rightValue);

                                if (result) {
                                    keyToDelete1.push(key);

                                }
                            }
                            else {
                                if (data == rightOperand1) {
                                    keyToDelete1.push(key);

                                }
                            }
                        }
                    }
                }
            }
        }

    }
    else {
        cout << "No match found." << endl;
    }
    if (regex_search(secondString, match2, pattern)) {
        leftOperand2 = match2[1];
        op2 = match2[2];
        rightOperand2 = match2[3];
        // ����TableData
        for (const auto& tableEntry : TableData) {
            if (tableEntry.first == tableName) {
                const auto& table = tableEntry.second;
                // �������е���
                for (const auto& columnEntry : table) {
                    const auto& column = columnEntry.second;
                    const string& columnName = columnEntry.first;
                    if (columnName == leftOperand2) {
                        // �������е���������������
                        for (const auto& primaryKeyEntry : column) {
                            const string& key = primaryKeyEntry.first;
                            const string& data = primaryKeyEntry.second;
                            if (findColumnType(tableName, leftOperand2) == "INT") {

                                // ���ַ���ת��Ϊ����
                                int leftValue = stoi(data);
                                int rightValue = stoi(rightOperand1);

                                // ʹ������ӳ��ִ�бȽ�
                                bool result = operators[op2](leftValue, rightValue);

                                if (result) {
                                    keyToDelete2.push(key);

                                }
                            }
                            else {
                                if (data == rightOperand2) {
                                    keyToDelete2.push(key);

                                }
                            }
                        }
                    }
                }
            }
        }

        if (operation == "OR") {
            // ��������
            while (!keyToDelete1.empty()) {
                string frontElement = keyToDelete1.front(); // ��ȡ��ͷԪ��
                DeleteTableRow(tableName, frontElement);
                keyToDelete1.pop();                      // ������ͷԪ��
            }
            // ��������
            while (!keyToDelete2.empty()) {
                string frontElement = keyToDelete2.front(); // ��ȡ��ͷԪ��
                DeleteTableRow(tableName, frontElement);
                keyToDelete2.pop();                      // ������ͷԪ��
            }
        }
        else if (operation == "AND") {
            while (!keyToDelete1.empty()) {
                string frontElement1 = keyToDelete1.front(); // ��ȡ��ͷԪ��
                queue<string> key2 = keyToDelete2;
                while (!key2.empty()) {
                    string frontElement2 = key2.front(); // ��ȡ��ͷԪ��
                    if (frontElement1 == frontElement2) {
                        DeleteTableRow(tableName, frontElement1);
                    }
                    key2.pop();                      // ������ͷԪ��
                }
                keyToDelete1.pop();                      // ������ͷԪ��
            }
        }
    }
    else {
        cout << "No match found." << endl;
    }
}

void DeleteFindWhere(string tableName, string condition) {

    // ���������ַ���
    string op;
    regex pattern(R"((\w+)\s*(=|>|<|>=|<=)\s*['"]?(\w+)['"]?)");
    smatch match;
    string leftOperand;
    string rightOperand;

    if (regex_search(condition, match, pattern)) {
        leftOperand = match[1];
        op = match[2];
        rightOperand = removeSingleQuotes(match[3]);
        queue<string> keyToDelete;
        // ����TableData

        for (const auto& tableEntry : TableData) {
            if (tableEntry.first == tableName) {
                const auto& table = tableEntry.second;
                // �������е���
                for (const auto& columnEntry : table) {
                    const auto& column = columnEntry.second;
                    string columnName = columnEntry.first;
                    if (columnName == leftOperand) {
                        // �������е���������������
                        for (const auto& primaryKeyEntry : column) {
                            string key = primaryKeyEntry.first;
                            string data = primaryKeyEntry.second;
                            if (findColumnType(tableName, leftOperand) == "INT") {

                                // ���ַ���ת��Ϊ����
                                int leftValue = stoi(data);
                                int rightValue = stoi(rightOperand);

                                // ʹ������ӳ��ִ�бȽ�
                                bool result = operators[op](leftValue, rightValue);

                                if (result) {
                                    keyToDelete.push(key);
                                }
                            }
                            else {
                                if (data == rightOperand) {
                                    keyToDelete.push(key);
                                }
                            }
                        }
                    }
                }
            }
        }
        // ��������
        while (!keyToDelete.empty()) {
            string frontElement = keyToDelete.front(); // ��ȡ��ͷԪ��
            DeleteTableRow(tableName, frontElement);
            keyToDelete.pop();                      // ������ͷԪ��
        }

    }
    else {
        cout << "No match found." << endl;
    }

}

//����ɾ������
void DeleteWhere(string tableName, string Where) {
    ReadTable(tableName);
    if (Where.find("AND") != string::npos || Where.find("OR") != string::npos) {
        DeleteFindWhereDouble(tableName, Where);
    }
    else {
        DeleteFindWhere(tableName, Where);
    }

    WriteTable(tableName);
}

//DeleteFrom���ɾ��
void DeleteFrom(string sql) {
    // ����һ��������ʽģʽ��ƥ��DELETE���
    regex pattern(R"((DELETE FROM) ([A-Za-z_][A-Za-z0-9_]*) (?:WHERE (.*))?;)");

    // �������ַ���������ƥ����
    smatch match;
    string deleteFrom;
    string tableName;
    string whereCondition;

    if (regex_search(sql, match, pattern)) {
        // match[0] ��������ƥ����ַ���
        // match[1] ���� "DELETE FROM"
        // match[2] ��������
        // match[3] ������ѡ��WHERE����
        deleteFrom = match[1];
        tableName = match[2];
        whereCondition = match[3];

        //�ж��Ƿ���where
        if (!whereCondition.empty()) {
            DeleteWhere(tableName, whereCondition);
        }
        else {
            WriteTable(tableName);
        }
    }
    else {
        cout << "No match found." << endl;
    }
}

//ɾ����
void DropTable(string sql) {
    // ����һ��������ʽģʽ��ƥ��DROP TABLE���
    regex pattern(R"(DROP TABLE ([A-Za-z_][A-Za-z0-9_]*);)");

    // �������ַ���������ƥ����
    smatch match;
    string tableName;
    if (regex_search(sql, match, pattern)) {
        // match[0] ��������ƥ����ַ���
        // match[1] ��������
        tableName = match[1];
    }
    else {
        cout << "No match found." << endl;
    }

    // ����Ҫɾ���ı��Ƿ����
    auto it = dataDictionary.find(tableName);

    if (it != dataDictionary.end()) {
        // ����ڣ�ɾ����
        dataDictionary.erase(it);
        cout << "�� " << tableName << " �ѱ�ɾ����" << endl;
    }
    else {
        // ������
        cout << "�� " << tableName << " �����ڣ��޷�ɾ����" << endl;
    }

    string Filename = tableName + ".txt"; // Ҫɾ�����ļ���

    const char* filename = Filename.c_str();

    if (remove(filename) == 0) {
        cout << "�ļ� " << filename << " �Ѿ���ɾ��" << endl;
    }
    else {
        cout << "�޷�ɾ���ļ� " << filename << endl;
    }
}

string ShowTable(string sql) {
    regex pattern(R"((SELECT \*) FROM ([A-Za-z_][A-Za-z0-9_]*);)");
    smatch match;

    if (regex_search(sql, match, pattern)) {
        string tableName = match[2];

        // ��ȡ��������
        ReadTable(tableName); // ȷ�� TableData �����

        // �����������
        string output = "Content of Table " + tableName + " :\r\n";
        const auto& tableData = TableData[tableName];

        if (tableData.empty()) {
            output += "Empty.\r\n";
        }
        else {
            for (const auto& columnEntry : tableData) {
                const string& columnName = columnEntry.first;
                output += "ColumnName: " + columnName + "\r\n";

                for (const auto& dataEntry : columnEntry.second) {
                    const string& key = dataEntry.first;
                    const string& value = dataEntry.second;
                    output += "Key: " + key + ", Value: " + value + "\r\n";
                }
                output += "\r\n"; // ÿ��֮��ӿ���
            }
        }

        return output; // ���ع����õ����
    }
    return "No Matching SQL Sentence";
}

void ChangeWithoutWHERE(string tableName, string propertyName, string propertyValue) {

    for (const auto& tableEntry : TableData) {
        if (tableName == tableEntry.first) {
            for (const auto& columnEntry : tableEntry.second) {
                string columnName = columnEntry.first;
                if (propertyName == columnName) {
                    for (const auto& rowEntry : columnEntry.second) {
                        string primaryKey = rowEntry.first;
                        TableData[tableName][columnName][primaryKey] = propertyValue;
                    }
                }
            }
        }
    }
}

void UpdataWithoutWHERE(string sql) {
    // ����һ��������ʽģʽ��ƥ��UPDATE���
    regex pattern(R"((UPDATE) ([A-Za-z_][A-Za-z0-9_]*) SET ([A-Za-z_][A-Za-z0-9_]*)=([^;]*);)");

    // �������ַ���������ƥ����
    smatch match;
    string update;
    string tableName;
    string columnName;
    string updatedValue;

    if (regex_search(sql, match, pattern)) {
        // match[0] ��������ƥ����ַ���
        // match[1] ���� "UPDATE"
        // match[2] ��������
        // match[3] ��������
        // match[4] �������µ�ֵ
        update = match[1];
        tableName = match[2];
        columnName = match[3];
        updatedValue = removeSingleQuotes(match[4]);
    }
    else {
        cout << "No match found." << std::endl;
    }
    cout << tableName << endl;
    ReadTable(tableName);
    ChangeWithoutWHERE(tableName, columnName, updatedValue);
    WriteTable(tableName);
}

void ChangeWithWHEREdouble(string tableName, string condition, string columnToSet, string valueToSet) {
    // ����һ��������ʽģʽ��ƥ��AND��OR������
    regex pattern0(R"(([^ ]+) (AND|OR) ([^ ]+))");
    string firstString;
    string operation;
    string secondString;

    // �������ַ���������ƥ����
    smatch match0;
    if (regex_search(condition, match0, pattern0)) {
        // match[0] ��������ƥ����ַ���
        // match[1] ������һ���ַ���
        // match[2] ����AND��OR������
        // match[3] �����ڶ����ַ���
        firstString = match0[1];
        operation = match0[2];
        secondString = match0[3];
        cout << firstString << endl << secondString << endl;
    }
    else {
        cout << "No match found." << endl;
    }

    // ���������ַ���
    string op1, op2;
    regex pattern(R"((\w+)\s*(=|>|<|>=|<=)\s*['"]?(\w+)['"]?)");
    smatch match1, match2;
    string leftOperand1, leftOperand2;
    string rightOperand1, rightOperand2;
    queue<string> keyToChange1, keyToChange2;
    if (regex_search(firstString, match1, pattern)) {
        leftOperand1 = match1[1];
        op1 = match1[2];
        rightOperand1 = match1[3];
        // ����TableData
        for (const auto& tableEntry : TableData) {

            if (tableEntry.first == tableName) {

                const auto& table = tableEntry.second;
                // �������е���
                for (const auto& columnEntry : table) {
                    const auto& column = columnEntry.second;
                    const string& columnName = columnEntry.first;
                    if (columnName == leftOperand1) {
                        // �������е���������������
                        for (const auto& primaryKeyEntry : column) {
                            const string& key = primaryKeyEntry.first;
                            const string& data = primaryKeyEntry.second;
                            if (findColumnType(tableName, leftOperand1) == "INT") {

                                // ���ַ���ת��Ϊ����
                                int leftValue = stoi(data);
                                int rightValue = stoi(rightOperand1);
                                // ʹ������ӳ��ִ�бȽ�
                                bool result = operators[op1](leftValue, rightValue);

                                if (result) {
                                    keyToChange1.push(key);

                                }
                            }
                            else {
                                if (data == rightOperand1) {
                                    keyToChange1.push(key);

                                }
                            }
                        }
                    }
                }
            }
        }

    }
    else {
        cout << "No match found." << endl;
    }

    if (regex_search(secondString, match2, pattern)) {
        leftOperand2 = match2[1];
        op2 = match2[2];
        rightOperand2 = match2[3];
        // ����TableData
        for (const auto& tableEntry : TableData) {
            if (tableEntry.first == tableName) {
                const auto& table = tableEntry.second;
                // �������е���
                for (const auto& columnEntry : table) {
                    const auto& column = columnEntry.second;
                    const string& columnName = columnEntry.first;
                    if (columnName == leftOperand2) {
                        // �������е���������������
                        for (const auto& primaryKeyEntry : column) {
                            const string& key = primaryKeyEntry.first;
                            const string& data = primaryKeyEntry.second;
                            if (findColumnType(tableName, leftOperand2) == "INT") {

                                // ���ַ���ת��Ϊ����
                                int leftValue = stoi(data);
                                int rightValue = stoi(rightOperand1);

                                // ʹ������ӳ��ִ�бȽ�
                                bool result = operators[op2](leftValue, rightValue);

                                if (result) {
                                    keyToChange2.push(key);

                                }
                            }
                            else {
                                if (data == rightOperand2) {
                                    keyToChange2.push(key);

                                }
                            }
                        }
                    }
                }
            }
        }

        if (operation == "OR") {

            // ��������
            while (!keyToChange1.empty()) {
                string frontElement = keyToChange1.front(); // ��ȡ��ͷԪ��
                TableData[tableName][columnToSet][frontElement] = valueToSet;
                cout << TableData[tableName][columnToSet][frontElement] << endl;
                keyToChange1.pop();                      // ������ͷԪ��
            }
            // ��������
            while (!keyToChange2.empty()) {
                string frontElement = keyToChange2.front(); // ��ȡ��ͷԪ��
                TableData[tableName][columnToSet][frontElement] = valueToSet;
                keyToChange2.pop();                      // ������ͷԪ��
            }
        }
        else if (operation == "AND") {
            while (!keyToChange1.empty()) {
                string frontElement1 = keyToChange1.front(); // ��ȡ��ͷԪ��
                queue<string> key2 = keyToChange2;
                while (!key2.empty()) {
                    string frontElement2 = key2.front(); // ��ȡ��ͷԪ��
                    if (frontElement1 == frontElement2) {
                        TableData[tableName][columnToSet][frontElement1] = valueToSet;
                    }
                    key2.pop();                      // ������ͷԪ��
                }
                keyToChange1.pop();                      // ������ͷԪ��
            }
        }
    }
    else {
        cout << "No match found." << endl;
    }
}

void ChangeWithWHEREsingle(string tableName, string condition, string columnToSet, string valueToSet) {
    // ���������ַ���
    string op;
    regex pattern(R"((\w+)\s*(=|>|<|>=|<=)\s*['"]?(\w+)['"]?)");
    smatch match;
    string leftOperand;
    string rightOperand;

    if (regex_search(condition, match, pattern)) {
        leftOperand = match[1];
        op = match[2];
        rightOperand = match[3];
        queue<string> keyToChange;
        // ����TableData

        for (const auto& tableEntry : TableData) {
            if (tableEntry.first == tableName) {
                const auto& table = tableEntry.second;
                // �������е���
                for (const auto& columnEntry : table) {
                    const auto& column = columnEntry.second;
                    const string& columnName = columnEntry.first;
                    if (columnName == leftOperand) {
                        // �������е���������������
                        for (const auto& primaryKeyEntry : column) {
                            const string& key = primaryKeyEntry.first;
                            const string& data = primaryKeyEntry.second;

                            if (findColumnType(tableName, leftOperand) == "INT") {
                                cout << op << endl;
                                // ���ַ���ת��Ϊ����
                                int leftValue = stoi(data);
                                int rightValue = stoi(rightOperand);

                                // ʹ������ӳ��ִ�бȽ�
                                bool result = operators[op](leftValue, rightValue);

                                if (result) {
                                    keyToChange.push(key);
                                }
                            }
                            else {
                                if (data == rightOperand) {
                                    keyToChange.push(key);

                                }
                            }
                        }
                    }
                }
            }
        }
        // ��������
        while (!keyToChange.empty()) {
            string frontElement = keyToChange.front(); // ��ȡ��ͷԪ��
            TableData[tableName][columnToSet][frontElement] = valueToSet;
            keyToChange.pop();                      // ������ͷԪ��
        }

    }
    else {
        cout << "No match found." << endl;
    }
}

void UpdataWithWHEREsingle(string tableName, string setValues, string whereClause) {
    // ����һ��������ʽģʽ��ƥ��Ⱥ������Ҳ���ַ���
    regex pattern(R"(([^=]+)=([^=]+))");


    // �������ַ���������ƥ����
    smatch match;
    string column = match[1];
    string value = match[2];
    if (regex_search(setValues, match, pattern)) {
        // match[0] ��������ƥ����ַ���
        // match[1] �����Ⱥ������ַ���
        // match[2] �����Ⱥ��Ҳ���ַ���
        column = match[1];
        value = removeSingleQuotes(match[2]);
    }
    else {
        cout << "No match found." << endl;
    }

    ChangeWithWHEREsingle(tableName, whereClause, column, value);
}

void UpdataWithWHEREdouble(string tableName, string setValues, string whereClause) {
    // ����һ��������ʽģʽ��ƥ��Ⱥ������Ҳ���ַ���
    regex pattern(R"(([^=]+)=([^=]+))");

    // �������ַ���������ƥ����
    smatch match;
    string column = match[1];
    string value = match[2];
    if (regex_search(setValues, match, pattern)) {
        // match[0] ��������ƥ����ַ���
        // match[1] �����Ⱥ������ַ���
        // match[2] �����Ⱥ��Ҳ���ַ���
        column = match[1];
        value = removeSingleQuotes(match[2]);
    }
    else {
        cout << "No match found." << endl;
    }

    ChangeWithWHEREdouble(tableName, whereClause, column, value);


}

void UpdataWithWHERE(string sql) {
    // ����һ��������ʽģʽ��ƥ��UPDATE���
    regex pattern(R"((UPDATE) ([A-Za-z_][A-Za-z0-9_]*).*SET (.*)\s*WHERE (.*);)");

    // �������ַ���������ƥ����
    smatch match;
    string update;
    string tableName;
    string setValues;
    string whereClause;
    if (regex_search(sql, match, pattern)) {
        // match[0] ��������ƥ����ַ���
        // match[1] ���� "UPDATE"
        // match[2] ��������
        // match[3] ������ֵ����
        // match[4] ������������
        update = match[1];
        tableName = match[2];
        setValues = match[3];
        whereClause = match[4];
    }
    else {
        cout << "No match found." << endl;
    }
    ReadTable(tableName);

    if (sql.find("AND") != string::npos || sql.find("OR") != string::npos) {
        UpdataWithWHEREdouble(tableName, setValues, whereClause);
    }
    else {
        UpdataWithWHEREsingle(tableName, setValues, whereClause);
    }
    WriteTable(tableName);
}

void Updata(string sql) {

    if (sql.find("WHERE") != string::npos) {
        UpdataWithWHERE(sql);
    }
    else {
        UpdataWithoutWHERE(sql);
    }
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
    // ������
    const wchar_t CLASS_NAME[] = L"SQL Executor Class"; // �޸�Ϊ wchar_t

    WNDCLASS wc = { };
    wc.lpfnWndProc = WindowProc;  // ���ڹ���
    wc.hInstance = hInstance;      // ����ʵ�����
    wc.lpszClassName = CLASS_NAME; // ����������
    wc.hCursor = LoadCursor(NULL, IDC_ARROW); // ���ù����ʽ

    // ע�ᴰ����
    RegisterClass(&wc);

    // ��������
    HWND hwnd = CreateWindowEx(
        0, CLASS_NAME, L"SQL Executor", WS_OVERLAPPEDWINDOW, // ȷ�����ڱ���Ҳ�ǿ��ַ�
        CW_USEDEFAULT, CW_USEDEFAULT, 600, 650, NULL, NULL, hInstance, NULL
    );

    // ��ʾ����
    ShowWindow(hwnd, nCmdShow);

    // ��ʼ�������ֵ�
    InitDataDictionary();
    PrintOutDataDictionary(hEditOutput);
    InitOperators();

    // ��Ϣѭ��
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}



/*
CREATE TABLE stu (sid CHAR(6),sname VARCHAR(20),age INT,gender VARCHAR(10),PRIMARY KEY(sid));
CREATE TABLE MyTable (column1 INT, column2 VARCHAR(255));
ALTER TABLE stu ADD (classname INT);
INSERT INTO stu VALUES ('s_1002','liSi',32,'male',4);
INSERT INTO stu (sid,sname,age,gender,classname) VALUES ('s_2003','JOE',23,'female',3);
ALTER TABLE stu DROP classname;
DELETE FROM stu WHERE sid='s_2003' ;
DELETE FROM stu WHERE age=23 ;
DELETE FROM stu WHERE sname='JOE' OR age=32 ;
DELETE FROM stu WHERE age>23 AND age<40 ;
DELETE FROM stu WHERE sid='s_2003' AND age=23 ;
UPDATE stu SET sname='zhangSanSan' WHERE sid='s_1002';
UPDATE stu SET age=18;
UPDATE stu SET age=18 WHERE age>23 AND age<40;
SELECT * FROM stu;
DELETE FROM stu ;
DROP TABLE stu;
*/
