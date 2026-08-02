#pragma once
#include <vector>
#include <string>
#include <mutex>

class TableVisualizer {
private:
    std::vector<std::string> columnHeaders;
    std::vector<std::string> rowHeaders;
    std::vector<std::vector<std::string>> cellData;
    std::mutex tableMutex;
    bool isCollapsed;
    char filterBuffer[256];

public:
    TableVisualizer();
    ~TableVisualizer() = default;

    static TableVisualizer& Get();

    void SetData(const std::vector<std::string>& cols,
                 const std::vector<std::string>& rows,
                 const std::vector<std::vector<std::string>>& cells);

    void SetHeaders(const std::vector<std::string>& cols,
                    const std::vector<std::string>& rows = {});

    void AddRow(const std::string& rowHeader, const std::vector<std::string>& rowData);
    void Clear();

    void Draw(const char* title = "Table Visualizer");

    bool IsCollapsed() const { return isCollapsed; }
    void SetCollapsed(bool collapsed) { isCollapsed = collapsed; }
};
