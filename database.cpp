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

// 函数声明
void CreateTable(string sql);//创建表
void InsertInto(string sql);//插入
void AlterTable(string sql);//修改表
void DeleteFrom(string sql);//删除
void Updata(string sql);//更新
void DropTable(string sql);//删除表
string ShowTable(string sql);//显示表

HWND hEditInput;   // SQL 输入框句柄
HWND hEditOutput;  // 输出结果框句柄

string ExecuteSQL(const wstring& sql) {
    string sqlStr(sql.begin(), sql.end());
    string result; // 用于捕获执行结果

    if (sqlStr.find("CREATE TABLE") != string::npos) {
        CreateTable(sqlStr);
        result = "Table created successfully."; // 设定结果字符串
    }
    else if (sqlStr.find("INSERT INTO") != string::npos) {
        InsertInto(sqlStr);
        result = "Row inserted successfully."; // 设定结果字符串
    }
    else if (sqlStr.find("ALTER TABLE") != string::npos) {
        AlterTable(sqlStr);
        result = "Table altered successfully."; // 设定结果字符串
    }
    else if (sqlStr.find("DELETE FROM") != string::npos) {
        DeleteFrom(sqlStr);
        result = "Row deleted successfully."; // 设定结果字符串
    }
    else if (sqlStr.find("UPDATE") != string::npos) {
        Updata(sqlStr);
        result = "Table updated successfully."; // 设定结果字符串
    }
    else if (sqlStr.find("DROP TABLE") != string::npos) {
        DropTable(sqlStr);
        result = "Table dropped successfully."; // 设定结果字符串
    }
    else if (sqlStr.find("SELECT * FROM") != string::npos) {
        result = ShowTable(sqlStr); // 直接获取显示内容
    }
    else {
        result = "没有匹配的 SQL 语句。"; // 设定错误结果字符串
    }

    return result;
}


LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_CREATE: {
        // 创建 SQL 输入框
        hEditInput = CreateWindowEx(
            WS_EX_CLIENTEDGE, L"EDIT", L"",
            WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE,
            10, 10, 560, 50, hwnd, NULL, GetModuleHandle(NULL), NULL
        );


        // 创建执行按钮
        CreateWindow(
            L"BUTTON", L"Execute",
            WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            10, 70, 100, 30, hwnd, (HMENU)1, GetModuleHandle(NULL), NULL
        );

		// 创建输出框
        hEditOutput = CreateWindowEx(
            WS_EX_CLIENTEDGE, L"EDIT", L"",
            WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_READONLY,
            10, 110, 560, 500, hwnd, NULL, GetModuleHandle(NULL), NULL
        );

        break;
    }

	// 执行逻辑
    case WM_COMMAND: {
        if (LOWORD(wParam) == 1) { // 如果点击了执行按钮
            wchar_t sql[256]; // 使用宽字符数组
            GetWindowText(hEditInput, sql, lstrlenW(sql) + 1); // 获取输入的 SQL 语句
            string result = ExecuteSQL(wstring(sql, sql + wcslen(sql))); // 获取执行结果字符串

            // 显示执行结果
            SetWindowText(hEditOutput, wstring(result.begin(), result.end()).c_str()); // 更新输出框
        }
        break;
    }

	 // 窗口销毁
    case WM_DESTROY: {
        PostQuitMessage(0);
        break;
    }
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}


// 数据表的列的结构体
struct Column {
    string name;            // 列名
    string type;            // 列的数据类型
    bool isPrimaryKey;      // 是否是主键
    bool isNullable;        // 是否可以为空
    bool isUnique;          // 是否唯一
};

// 数据表的结构体
struct Table {
    string name;            // 表名
    vector<Column> columns; // 列的集合
};

// 数据字典，以表名作为键，存储表的元信息
map<string, Table> dataDictionary;

//数据库中各表存储的内容 表名 列名 主键 数据内容
map<string, map<string, map<string, string>>> TableData;

//存储查询结果
queue<string> out;

// 创建一个映射，将操作符映射到相应的函数对象
map<string, function<bool(int, int)>> operators;

// 函数来查找某一列的数据类型
string findColumnType(string tableName, string columnName) {
    // 遍历dataDictionary
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
    return "Column not found"; // 或者返回适当的错误值
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

    // 遍历表名
    for (const auto& tableEntry : TableData) {
        const string& tableName = tableEntry.first;
        output += "表名: " + tableName + "\r\n";

        // 遍历列名
        for (const auto& columnEntry : tableEntry.second) {
            const string& columnName = columnEntry.first;
            output += "  列名: " + columnName + "\r\n";

            // 遍历主键和数据内容
            for (const auto& dataEntry : columnEntry.second) {
                const string& key = dataEntry.first;
                const string& value = dataEntry.second;

                if (key == "主键") {
                    output += "    主键: " + value + "\r\n";
                }
                else {
                    output += "    数据内容: " + key + " = " + value + "\r\n";
                }
            }
        }
        output += "\r\n"; // 每个表之间加一个空行
    }

    // 将结果显示在输出框中
    SetWindowText(hEditOutput, wstring(output.begin(), output.end()).c_str());
}



void PrintOutDataDictionary(HWND hEditOutput) {
    string output;

    for (const auto& entry : dataDictionary) {
        const string& tableName = entry.first;
        const Table& tableInfo = entry.second;

        output += "TableName: " + tableName + "\r\n";

        // 输出列信息
        for (const Column& column : tableInfo.columns) {
            output += "ColumnName: " + column.name + "\r\n";
            output += "DateType: " + column.type + "\r\n";
            output += "ISKey: " + string(column.isPrimaryKey ? "Yes" : "No") + "\r\n";
            output += "ISNULL: " + string(column.isNullable ? "Yes" : "No") + "\r\n";
            output += "ISONLY: " + string(column.isUnique ? "Yes" : "No") + "\r\n";
            output += "\r\n"; // 每个表信息之间加一个空行
        }
    }

    // 将结果显示在输出框中
    SetWindowText(hEditOutput, wstring(output.begin(), output.end()).c_str());
}


//写入数据表
void WriteTable(string tableName) {
    //写入map 主键 列名 数据内容
    map<string, map<string, string>> write;

    PrintOutDataDictionary(hEditOutput);

    // 遍历TableData
    for (const auto& tableEntry : TableData) {
        if (tableEntry.first == tableName) {
            const auto& table = tableEntry.second;
            // 遍历表中的列
            for (const auto& columnEntry : table) {
                const auto& column = columnEntry.second;
                const string& columnName = columnEntry.first;

                // 遍历列中的主键和数据内容
                for (const auto& primaryKeyEntry : column) {
                    const string& key = primaryKeyEntry.first;
                    const string& data = primaryKeyEntry.second;
                    write[key][columnName] = data;
                }
            }
        }
    }

    // 打开文件以覆盖写入内容
    ofstream file(tableName + ".txt");

    if (!file.is_open()) {
        cerr << "无法打开文件" << endl;
    }
    // 使用范围循环遍历write map
    for (const auto& entry : write) {
        string key = entry.first;

        // 创建一个临时队列来保存原队列中的元素
        queue<string> tempQueue = out;
        string line;
        // 遍历原队列
        while (!tempQueue.empty()) {
            string frontElement = tempQueue.front(); // 获取队头元素

            if (!write[key][frontElement].empty()) {
                line += write[key][frontElement] + " ";
                cout << frontElement << " " << write[key][frontElement] << endl;
            }
            tempQueue.pop();                        // 弹出队头元素
        }

        file << line << endl;
    }

    // 关闭文件
    file.close();

}

void ReadTable(string tableName) {
    ifstream inputFile(tableName + ".txt");
    if (!inputFile) {
        string errorOutput = "无法打开文件: " + tableName + ".txt\r\n";
        SetWindowText(hEditOutput, wstring(errorOutput.begin(), errorOutput.end()).c_str());
        return;
    }

    string line;
    while (getline(inputFile, line)) {
        istringstream iss(line);
        vector<string> words;
        string word;

        // 使用空格分割行
        while (iss >> word) {
            words.push_back(word);
        }

        // 在数据字典中查找表格
        auto tableIt = dataDictionary.find(tableName);
        if (tableIt != dataDictionary.end()) {
            const Table& table = tableIt->second;

            // 假设第一列为主键
            const string& primaryKey = words[0];
            for (size_t i = 0; i < table.columns.size() && i < words.size(); i++) {
                const Column& column = table.columns[i];
                TableData[tableName][column.name][primaryKey] = words[i];
            }
        }
    }
    inputFile.close();
}



// 函数以引用方式接受一个字符串，从该字符串中删除所有的单引号
string removeSingleQuotes(string str) {
    size_t pos = str.find('\'');
    while (pos != string::npos) {
        str.erase(pos, 1); // 从字符串中删除一个字符
        pos = str.find('\'', pos); // 继续查找下一个单引号
    }
    return str;
}

//int转化为字符串
string IntToString(int num) {
    string str = to_string(num);
    return str;
}

//字符串转化为int
int StringToInt(string str) {
    int num = stoi(str);
    return num;
}


//创建指定名字的.txt文件
void CreateFile(string filename) {

    // 创建一个输出文件流对象
    ofstream outFile(filename + ".txt");

    if (outFile.is_open()) {
        // 关闭文件
        outFile.close();
    }
}

//判断真假
bool Judge(string str) {
    if (str == "true") {
        return true;
    }
    else {
        return false;
    }
}

//判断真假
string JudgeBack(bool judge) {
    if (judge) return "true";
    else return "false";
}

//提取括号中信息
string extractContentInParentheses(const string& input) {
    size_t startPos = input.find('(');
    size_t endPos = input.find(')', startPos);

    if (startPos != string::npos && endPos != string::npos) {
        return input.substr(startPos + 1, endPos - startPos - 1);
    }
    else {
        return ""; // 如果未找到括号内容
    }
}

//处理数据字典中的表语句
void AddTable(string str) {
    Table newTable;
    // 将新表添加到数据字典中
    dataDictionary[str] = newTable;
}

//处理数据字典中的列语句
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

//读取数据字典
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

// 将表的指定列设置为主键
void setColumnAsPrimaryKey(const string& tableName, const string& columnName) {
    // 首先检查表是否存在于数据字典中
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

// 找主键
string FindPrimaryKey(const string& tableName) {
    // 首先检查表是否存在于数据字典中
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

//create table建立数据库表的列属性
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

//create table建立数据库表
void CreateTable(string sql) {
    // 创建一个正则表达式模式来匹配CREATE TABLE语句
    regex pattern(R"((CREATE TABLE) ([A-Za-z_][A-Za-z0-9_]*) \(([^;]*)\);)");

    // 在输入字符串中搜索匹配项
    smatch match;
    string createTable;
    string tableName;
    string columnInfo;
    if (regex_search(sql, match, pattern)) {
        // match[0] 包含整个匹配的字符串
        // match[1] 包含 "CREATE TABLE"
        // match[2] 包含表名
        // match[3] 包含列信息
        createTable = match[1];
        tableName = match[2];
        columnInfo = match[3];
    }
    else {
        cout << "未能成功匹配！" << endl;
    }
	AddTable(tableName);//将表名添加到数据字典
	vector<string> result;
    istringstream read(columnInfo);
    string token;
	// 以逗号分隔列名
    while (getline(read, token, ',')) {
        result.push_back(token);
    }
	// 将列名添加到数据字典
    for (const string& str : result) {
        CreateColumn(tableName, str);
    }
    CreateFile(tableName);
}


//从文件读取数据字典
void InitDataDictionary() {
    // 打开文件以进行读取
    ifstream inputFile("dataDictionary.txt");

    if (!inputFile.is_open()) {
        // 如果文件不存在，创建一个新文件
        ofstream outputFile("dataDictionary.txt");
        if (outputFile.is_open()) {
            cout << "数据库不存在，已创建数据库" << endl;
            // 可以在这里写入新文件的内容
            outputFile << "这是一个新创建的数据库。\n";
            outputFile.close();
        }
        else {
            cerr << "无法创建数据库'" << endl;
        }
    }
    else {
        string line;
        while (getline(inputFile, line)) {
            // 处理数据字典文件中的内容
            ReadDataDictionary(line);
        }
        inputFile.close();
    }
}


//写入数据
void WriteDataDictionary() {
    // 打开文件并设置文件模式为输出和覆盖
    ofstream outputFile("DataDictionary.txt", ios::out | ios::trunc);
    if (!outputFile.is_open()) {
        cerr << "无法打开文件 " << "DataDictionary.txt" << endl;
    }

    // 遍历每个表的信息
    for (const auto& entry : dataDictionary) {
        const string& tableName = entry.first;
        const Table& tableInfo = entry.second;
        string write;
        write = tableName;

        // 遍历列信息
        for (const Column& column : tableInfo.columns) {
            write += "/" + column.name + " " + column.type + " " + (column.isPrimaryKey ? "true" : "false") + " " + (column.isNullable ? "true" : "false") + " " + (column.isUnique ? "true" : "false");

        }
        outputFile << write << endl;
    }
    // 关闭文件
    outputFile.close();
}

// 有列名列表的INSERT INTO语句
void InsertIntoWithColumns(string sql) {
    // 创建一个正则表达式模式来匹配INSERT INTO语句
    regex pattern(R"((INSERT INTO) ([A-Za-z_][A-Za-z0-9_]*) \(([^)]*)\) VALUES \(([^)]*)\);)");

    string insertInto;
    string tableName;
    string columnNames;
    string values;

    // 在输入字符串中搜索匹配项
    smatch match;
    if (regex_search(sql, match, pattern)) {
        // match[0] 包含整个匹配的字符串
        // match[1] 包含 "INSERT INTO"
        // match[2] 包含表名
        // match[3] 包含列名列表
        // match[4] 包含值列表
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

    // 打开文件并设置文件模式为输出和追加
    ofstream outputFile(tableName + ".txt", ios::out | ios::app);

    if (!outputFile.is_open()) {
        cerr << "无法打开文件 " << tableName + ".txt" << endl;
    }

    for (size_t i = 0; i < columnresult.size(); i++) {
        string ColumnName = removeSingleQuotes(columnresult[i]);
        string Value = removeSingleQuotes(valueresult[i]);
        //在数据字典中查找该表是否有该列
        auto tableIt = dataDictionary.find(tableName);
        if (tableIt != dataDictionary.end()) {
            const Table& table = tableIt->second;
            for (const Column& column : table.columns) {
                if (column.name == ColumnName) {
                    // 找到列名

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

    // 逐行写入文件（这次将不覆盖以前的内容）
    outputFile << line << endl;

    // 关闭文件
    outputFile.close();
}

// 没有列名列表的INSERT INTO语句
void InsertIntoWithoutColumns(string sql) {

    // 创建一个正则表达式模式来匹配INSERT INTO语句
    regex pattern(R"((INSERT INTO) ([A-Za-z_][A-Za-z0-9_]*) (?:\([^)]*\) )?VALUES \(([^)]*)\);)");

    string insertInto;
    string tableName;
    string columnNames;
    string values;

    // 在输入字符串中搜索匹配项
    smatch match;
    if (regex_search(sql, match, pattern)) {
        // match[0] 包含整个匹配的字符串
        // match[1] 包含 "INSERT INTO"
        // match[2] 包含表名
        // match[3] 包含值列表
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

    // 打开文件并设置文件模式为输出和追加
    ofstream outputFile(tableName + ".txt", ios::out | ios::app);

    if (!outputFile.is_open()) {
        cerr << "无法打开文件 " << tableName + ".txt" << endl;
    }

    string line;

    for (const string& str : result) {
        string Str = removeSingleQuotes(str);
        line += Str + " ";
    }

    // 逐行写入文件（这次将不覆盖以前的内容）
    outputFile << line << endl;

    // 关闭文件
    outputFile.close();

}


//InsertInto为关系表插入元组
void InsertInto(string sql) {
    // 创建一个正则表达式模式来匹配INSERT INTO语句
    regex patternWithColumns(R"((INSERT INTO) ([A-Za-z_][A-Za-z0-9_]*) \(([^)]*)\) VALUES \(([^)]*)\);)");
    regex patternWithoutColumns(R"((INSERT INTO) ([A-Za-z_][A-Za-z0-9_]*) VALUES \(([^)]*)\);)");

    // 在输入字符串中搜索匹配项
    smatch matchWithColumns;
    smatch matchWithoutColumns;

    if (regex_search(sql, matchWithColumns, patternWithColumns)) {
        // 匹配到有列名列表的INSERT INTO语句
        InsertIntoWithColumns(sql);
    }
    else if (regex_search(sql, matchWithoutColumns, patternWithoutColumns)) {
        // 匹配到没有列名列表的INSERT INTO语句
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

//AlterADD添加属性
void AlterADD(string sql) {

    // 创建一个正则表达式模式来匹配ALTER TABLE语句
    regex pattern(R"((ALTER TABLE) ([A-Za-z_][A-Za-z0-9_]*) ADD \(([^;]*)\);)");

    // 在输入字符串中搜索匹配项
    smatch match;
    string alterTable;
    string tableName;
    string columnInfo;

    if (regex_search(sql, match, pattern)) {
        // match[0] 包含整个匹配的字符串
        // match[1] 包含 "ALTER TABLE"
        // match[2] 包含表名
        // match[3] 包含要添加的列信息
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



// 函数：从数据表中删除指定列
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




//删除列元祖
void DeleteColumn(string tableName, string columnName) {
    ReadTable(tableName);

    //在TableData中删除
    auto tableIt = TableData.find(tableName);
    if (tableIt != TableData.end()) {
        auto columnIt = tableIt->second.find(columnName);
        if (columnIt != tableIt->second.end()) {
            tableIt->second.erase(columnIt);
            cout << "整列已删除" << endl;
        }
    }

    //在数据字典中删除
    deleteColumnFromTable(tableName, columnName);

    //重新写入文件
    WriteTable(tableName);
}

//AlterDROP删除属性
void AlterDROP(string sql) {

    // 创建一个正则表达式模式来匹配ALTER TABLE语句中的DROP操作
    regex pattern(R"((ALTER TABLE) ([A-Za-z_][A-Za-z0-9_]*) (DROP) ([A-Za-z_][A-Za-z0-9_]*);)");

    // 在输入字符串中搜索匹配项
    smatch match;
    string alterTable;
    string tableName;
    string operation;
    string columnName;

    if (regex_search(sql, match, pattern)) {
        // match[0] 包含整个匹配的字符串
        // match[1] 包含 "ALTER TABLE"
        // match[2] 包含表名
        // match[3] 包含 "DROP"
        // match[4] 包含要删除的列名
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
        cout << "主键不可删除！" << endl;
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

// 遍历嵌套的map并打印内容
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

// 删除特定行的函数
void DeleteTableRow(string tableName, string primaryKey) {
    TraverseNestedMap();
    // 遍历TableData中的每个表格
    for (auto& table : TableData) {
        // 遍历当前表格中的每一列
        for (auto& column : table.second) {
            // 检查当前列中是否包含指定的主键
            auto primaryKeyIt = column.second.find(primaryKey);
            if (primaryKeyIt != column.second.end()) {
                // 如果找到主键，从列中删除它
                column.second.erase(primaryKey);

                // 输出成功删除主键的信息，包括表格、列和主键的标识
                cout << "成功删除了行 " << table.first << " - " << column.first << " - " << primaryKey << endl;
            }
        }
    }
    TraverseNestedMap();
}

void DeleteFindWhereDouble(string tableName, string condition) {

    // 创建一个正则表达式模式来匹配AND或OR操作符
    regex pattern0(R"(([^ ]+) (AND|OR) ([^ ]+))");
    string firstString;
    string operation;
    string secondString;

    // 在输入字符串中搜索匹配项
    smatch match0;
    if (std::regex_search(condition, match0, pattern0)) {
        // match[0] 包含整个匹配的字符串
        // match[1] 包含第一个字符串
        // match[2] 包含AND或OR操作符
        // match[3] 包含第二个字符串
        firstString = match0[1];
        operation = match0[2];
        secondString = match0[3];
    }
    else {
        cout << "No match found." << endl;
    }

    // 解析条件字符串
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
        // 遍历TableData
        for (const auto& tableEntry : TableData) {
            if (tableEntry.first == tableName) {
                const auto& table = tableEntry.second;
                // 遍历表中的列
                for (const auto& columnEntry : table) {
                    const auto& column = columnEntry.second;
                    const string& columnName = columnEntry.first;
                    if (columnName == leftOperand1) {
                        // 遍历列中的主键和数据内容
                        for (const auto& primaryKeyEntry : column) {
                            const string& key = primaryKeyEntry.first;
                            const string& data = primaryKeyEntry.second;
                            if (findColumnType(tableName, leftOperand1) == "INT") {

                                // 将字符串转换为整数
                                int leftValue = stoi(data);
                                int rightValue = stoi(rightOperand1);

                                // 使用条件映射执行比较
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
        // 遍历TableData
        for (const auto& tableEntry : TableData) {
            if (tableEntry.first == tableName) {
                const auto& table = tableEntry.second;
                // 遍历表中的列
                for (const auto& columnEntry : table) {
                    const auto& column = columnEntry.second;
                    const string& columnName = columnEntry.first;
                    if (columnName == leftOperand2) {
                        // 遍历列中的主键和数据内容
                        for (const auto& primaryKeyEntry : column) {
                            const string& key = primaryKeyEntry.first;
                            const string& data = primaryKeyEntry.second;
                            if (findColumnType(tableName, leftOperand2) == "INT") {

                                // 将字符串转换为整数
                                int leftValue = stoi(data);
                                int rightValue = stoi(rightOperand1);

                                // 使用条件映射执行比较
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
            // 遍历队列
            while (!keyToDelete1.empty()) {
                string frontElement = keyToDelete1.front(); // 获取队头元素
                DeleteTableRow(tableName, frontElement);
                keyToDelete1.pop();                      // 弹出队头元素
            }
            // 遍历队列
            while (!keyToDelete2.empty()) {
                string frontElement = keyToDelete2.front(); // 获取队头元素
                DeleteTableRow(tableName, frontElement);
                keyToDelete2.pop();                      // 弹出队头元素
            }
        }
        else if (operation == "AND") {
            while (!keyToDelete1.empty()) {
                string frontElement1 = keyToDelete1.front(); // 获取队头元素
                queue<string> key2 = keyToDelete2;
                while (!key2.empty()) {
                    string frontElement2 = key2.front(); // 获取队头元素
                    if (frontElement1 == frontElement2) {
                        DeleteTableRow(tableName, frontElement1);
                    }
                    key2.pop();                      // 弹出队头元素
                }
                keyToDelete1.pop();                      // 弹出队头元素
            }
        }
    }
    else {
        cout << "No match found." << endl;
    }
}

void DeleteFindWhere(string tableName, string condition) {

    // 解析条件字符串
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
        // 遍历TableData

        for (const auto& tableEntry : TableData) {
            if (tableEntry.first == tableName) {
                const auto& table = tableEntry.second;
                // 遍历表中的列
                for (const auto& columnEntry : table) {
                    const auto& column = columnEntry.second;
                    string columnName = columnEntry.first;
                    if (columnName == leftOperand) {
                        // 遍历列中的主键和数据内容
                        for (const auto& primaryKeyEntry : column) {
                            string key = primaryKeyEntry.first;
                            string data = primaryKeyEntry.second;
                            if (findColumnType(tableName, leftOperand) == "INT") {

                                // 将字符串转换为整数
                                int leftValue = stoi(data);
                                int rightValue = stoi(rightOperand);

                                // 使用条件映射执行比较
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
        // 遍历队列
        while (!keyToDelete.empty()) {
            string frontElement = keyToDelete.front(); // 获取队头元素
            DeleteTableRow(tableName, frontElement);
            keyToDelete.pop();                      // 弹出队头元素
        }

    }
    else {
        cout << "No match found." << endl;
    }

}

//分析删除条件
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

//DeleteFrom语句删除
void DeleteFrom(string sql) {
    // 创建一个正则表达式模式来匹配DELETE语句
    regex pattern(R"((DELETE FROM) ([A-Za-z_][A-Za-z0-9_]*) (?:WHERE (.*))?;)");

    // 在输入字符串中搜索匹配项
    smatch match;
    string deleteFrom;
    string tableName;
    string whereCondition;

    if (regex_search(sql, match, pattern)) {
        // match[0] 包含整个匹配的字符串
        // match[1] 包含 "DELETE FROM"
        // match[2] 包含表名
        // match[3] 包含可选的WHERE条件
        deleteFrom = match[1];
        tableName = match[2];
        whereCondition = match[3];

        //判断是否有where
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

//删除表
void DropTable(string sql) {
    // 创建一个正则表达式模式来匹配DROP TABLE语句
    regex pattern(R"(DROP TABLE ([A-Za-z_][A-Za-z0-9_]*);)");

    // 在输入字符串中搜索匹配项
    smatch match;
    string tableName;
    if (regex_search(sql, match, pattern)) {
        // match[0] 包含整个匹配的字符串
        // match[1] 包含表名
        tableName = match[1];
    }
    else {
        cout << "No match found." << endl;
    }

    // 查找要删除的表是否存在
    auto it = dataDictionary.find(tableName);

    if (it != dataDictionary.end()) {
        // 表存在，删除它
        dataDictionary.erase(it);
        cout << "表 " << tableName << " 已被删除。" << endl;
    }
    else {
        // 表不存在
        cout << "表 " << tableName << " 不存在，无法删除。" << endl;
    }

    string Filename = tableName + ".txt"; // 要删除的文件名

    const char* filename = Filename.c_str();

    if (remove(filename) == 0) {
        cout << "文件 " << filename << " 已经被删除" << endl;
    }
    else {
        cout << "无法删除文件 " << filename << endl;
    }
}

string ShowTable(string sql) {
    regex pattern(R"((SELECT \*) FROM ([A-Za-z_][A-Za-z0-9_]*);)");
    smatch match;

    if (regex_search(sql, match, pattern)) {
        string tableName = match[2];

        // 读取表中数据
        ReadTable(tableName); // 确保 TableData 被填充

        // 构建输出内容
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
                output += "\r\n"; // 每列之间加空行
            }
        }

        return output; // 返回构建好的输出
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
    // 创建一个正则表达式模式来匹配UPDATE语句
    regex pattern(R"((UPDATE) ([A-Za-z_][A-Za-z0-9_]*) SET ([A-Za-z_][A-Za-z0-9_]*)=([^;]*);)");

    // 在输入字符串中搜索匹配项
    smatch match;
    string update;
    string tableName;
    string columnName;
    string updatedValue;

    if (regex_search(sql, match, pattern)) {
        // match[0] 包含整个匹配的字符串
        // match[1] 包含 "UPDATE"
        // match[2] 包含表名
        // match[3] 包含列名
        // match[4] 包含更新的值
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
    // 创建一个正则表达式模式来匹配AND或OR操作符
    regex pattern0(R"(([^ ]+) (AND|OR) ([^ ]+))");
    string firstString;
    string operation;
    string secondString;

    // 在输入字符串中搜索匹配项
    smatch match0;
    if (regex_search(condition, match0, pattern0)) {
        // match[0] 包含整个匹配的字符串
        // match[1] 包含第一个字符串
        // match[2] 包含AND或OR操作符
        // match[3] 包含第二个字符串
        firstString = match0[1];
        operation = match0[2];
        secondString = match0[3];
        cout << firstString << endl << secondString << endl;
    }
    else {
        cout << "No match found." << endl;
    }

    // 解析条件字符串
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
        // 遍历TableData
        for (const auto& tableEntry : TableData) {

            if (tableEntry.first == tableName) {

                const auto& table = tableEntry.second;
                // 遍历表中的列
                for (const auto& columnEntry : table) {
                    const auto& column = columnEntry.second;
                    const string& columnName = columnEntry.first;
                    if (columnName == leftOperand1) {
                        // 遍历列中的主键和数据内容
                        for (const auto& primaryKeyEntry : column) {
                            const string& key = primaryKeyEntry.first;
                            const string& data = primaryKeyEntry.second;
                            if (findColumnType(tableName, leftOperand1) == "INT") {

                                // 将字符串转换为整数
                                int leftValue = stoi(data);
                                int rightValue = stoi(rightOperand1);
                                // 使用条件映射执行比较
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
        // 遍历TableData
        for (const auto& tableEntry : TableData) {
            if (tableEntry.first == tableName) {
                const auto& table = tableEntry.second;
                // 遍历表中的列
                for (const auto& columnEntry : table) {
                    const auto& column = columnEntry.second;
                    const string& columnName = columnEntry.first;
                    if (columnName == leftOperand2) {
                        // 遍历列中的主键和数据内容
                        for (const auto& primaryKeyEntry : column) {
                            const string& key = primaryKeyEntry.first;
                            const string& data = primaryKeyEntry.second;
                            if (findColumnType(tableName, leftOperand2) == "INT") {

                                // 将字符串转换为整数
                                int leftValue = stoi(data);
                                int rightValue = stoi(rightOperand1);

                                // 使用条件映射执行比较
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

            // 遍历队列
            while (!keyToChange1.empty()) {
                string frontElement = keyToChange1.front(); // 获取队头元素
                TableData[tableName][columnToSet][frontElement] = valueToSet;
                cout << TableData[tableName][columnToSet][frontElement] << endl;
                keyToChange1.pop();                      // 弹出队头元素
            }
            // 遍历队列
            while (!keyToChange2.empty()) {
                string frontElement = keyToChange2.front(); // 获取队头元素
                TableData[tableName][columnToSet][frontElement] = valueToSet;
                keyToChange2.pop();                      // 弹出队头元素
            }
        }
        else if (operation == "AND") {
            while (!keyToChange1.empty()) {
                string frontElement1 = keyToChange1.front(); // 获取队头元素
                queue<string> key2 = keyToChange2;
                while (!key2.empty()) {
                    string frontElement2 = key2.front(); // 获取队头元素
                    if (frontElement1 == frontElement2) {
                        TableData[tableName][columnToSet][frontElement1] = valueToSet;
                    }
                    key2.pop();                      // 弹出队头元素
                }
                keyToChange1.pop();                      // 弹出队头元素
            }
        }
    }
    else {
        cout << "No match found." << endl;
    }
}

void ChangeWithWHEREsingle(string tableName, string condition, string columnToSet, string valueToSet) {
    // 解析条件字符串
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
        // 遍历TableData

        for (const auto& tableEntry : TableData) {
            if (tableEntry.first == tableName) {
                const auto& table = tableEntry.second;
                // 遍历表中的列
                for (const auto& columnEntry : table) {
                    const auto& column = columnEntry.second;
                    const string& columnName = columnEntry.first;
                    if (columnName == leftOperand) {
                        // 遍历列中的主键和数据内容
                        for (const auto& primaryKeyEntry : column) {
                            const string& key = primaryKeyEntry.first;
                            const string& data = primaryKeyEntry.second;

                            if (findColumnType(tableName, leftOperand) == "INT") {
                                cout << op << endl;
                                // 将字符串转换为整数
                                int leftValue = stoi(data);
                                int rightValue = stoi(rightOperand);

                                // 使用条件映射执行比较
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
        // 遍历队列
        while (!keyToChange.empty()) {
            string frontElement = keyToChange.front(); // 获取队头元素
            TableData[tableName][columnToSet][frontElement] = valueToSet;
            keyToChange.pop();                      // 弹出队头元素
        }

    }
    else {
        cout << "No match found." << endl;
    }
}

void UpdataWithWHEREsingle(string tableName, string setValues, string whereClause) {
    // 创建一个正则表达式模式来匹配等号左侧和右侧的字符串
    regex pattern(R"(([^=]+)=([^=]+))");


    // 在输入字符串中搜索匹配项
    smatch match;
    string column = match[1];
    string value = match[2];
    if (regex_search(setValues, match, pattern)) {
        // match[0] 包含整个匹配的字符串
        // match[1] 包含等号左侧的字符串
        // match[2] 包含等号右侧的字符串
        column = match[1];
        value = removeSingleQuotes(match[2]);
    }
    else {
        cout << "No match found." << endl;
    }

    ChangeWithWHEREsingle(tableName, whereClause, column, value);
}

void UpdataWithWHEREdouble(string tableName, string setValues, string whereClause) {
    // 创建一个正则表达式模式来匹配等号左侧和右侧的字符串
    regex pattern(R"(([^=]+)=([^=]+))");

    // 在输入字符串中搜索匹配项
    smatch match;
    string column = match[1];
    string value = match[2];
    if (regex_search(setValues, match, pattern)) {
        // match[0] 包含整个匹配的字符串
        // match[1] 包含等号左侧的字符串
        // match[2] 包含等号右侧的字符串
        column = match[1];
        value = removeSingleQuotes(match[2]);
    }
    else {
        cout << "No match found." << endl;
    }

    ChangeWithWHEREdouble(tableName, whereClause, column, value);


}

void UpdataWithWHERE(string sql) {
    // 创建一个正则表达式模式来匹配UPDATE语句
    regex pattern(R"((UPDATE) ([A-Za-z_][A-Za-z0-9_]*).*SET (.*)\s*WHERE (.*);)");

    // 在输入字符串中搜索匹配项
    smatch match;
    string update;
    string tableName;
    string setValues;
    string whereClause;
    if (regex_search(sql, match, pattern)) {
        // match[0] 包含整个匹配的字符串
        // match[1] 包含 "UPDATE"
        // match[2] 包含表名
        // match[3] 包含列值设置
        // match[4] 包含更新条件
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
    // 窗口类
    const wchar_t CLASS_NAME[] = L"SQL Executor Class"; // 修改为 wchar_t

    WNDCLASS wc = { };
    wc.lpfnWndProc = WindowProc;  // 窗口过程
    wc.hInstance = hInstance;      // 程序实例句柄
    wc.lpszClassName = CLASS_NAME; // 窗口类名称
    wc.hCursor = LoadCursor(NULL, IDC_ARROW); // 设置光标样式

    // 注册窗口类
    RegisterClass(&wc);

    // 创建窗口
    HWND hwnd = CreateWindowEx(
        0, CLASS_NAME, L"SQL Executor", WS_OVERLAPPEDWINDOW, // 确保窗口标题也是宽字符
        CW_USEDEFAULT, CW_USEDEFAULT, 600, 650, NULL, NULL, hInstance, NULL
    );

    // 显示窗口
    ShowWindow(hwnd, nCmdShow);

    // 初始化数据字典
    InitDataDictionary();
    PrintOutDataDictionary(hEditOutput);
    InitOperators();

    // 消息循环
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
