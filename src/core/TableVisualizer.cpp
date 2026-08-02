#include "TableVisualizer.hpp"
#include <cstdio>
#include "imgui/imgui.h"

TableVisualizer::TableVisualizer()
    : isCollapsed(false) {
    filterBuffer[0] = '\0';
}

TableVisualizer& TableVisualizer::Get() {
    static TableVisualizer instance;
    return instance;
}

void TableVisualizer::SetData(const std::vector<std::string>& cols,
                             const std::vector<std::string>& rows,
                             const std::vector<std::vector<std::string>>& cells) {
    std::lock_guard<std::mutex> lock(tableMutex);
    columnHeaders = cols;
    rowHeaders = rows;
    cellData = cells;
}

void TableVisualizer::SetHeaders(const std::vector<std::string>& cols,
                                const std::vector<std::string>& rows) {
    std::lock_guard<std::mutex> lock(tableMutex);
    columnHeaders = cols;
    rowHeaders = rows;
}

void TableVisualizer::AddRow(const std::string& rowHeader, const std::vector<std::string>& rowData) {
    std::lock_guard<std::mutex> lock(tableMutex);
    rowHeaders.push_back(rowHeader);
    cellData.push_back(rowData);
}

void TableVisualizer::Clear() {
    std::lock_guard<std::mutex> lock(tableMutex);
    columnHeaders.clear();
    rowHeaders.clear();
    cellData.clear();
}

void TableVisualizer::Draw(const char* title) {
    std::lock_guard<std::mutex> lock(tableMutex);

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4.0f, 2.0f));

    ImGui::TextUnformatted(title);
    ImGui::SameLine();
    ImGui::TextDisabled("(%d rows x %d cols)", (int)cellData.size(), (int)columnHeaders.size());

    if (!isCollapsed) {
        ImGui::SameLine();
        ImGui::SetNextItemWidth(120.0f);
        ImGui::InputText("##TableFilter", filterBuffer, sizeof(filterBuffer));

        ImGui::SameLine();
        if (ImGui::Button("Clear Table")) {
            columnHeaders.clear();
            rowHeaders.clear();
            cellData.clear();
        }

        ImGui::Separator();

        if (columnHeaders.empty() && cellData.empty()) {
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "No table data supplied.");
            ImGui::PopStyleVar();
            return;
        }

        bool hasRowHeaders = !rowHeaders.empty();
        int totalCols = (int)columnHeaders.size() + (hasRowHeaders ? 1 : 0);

        ImGuiTableFlags tableFlags = ImGuiTableFlags_Borders | 
                                     ImGuiTableFlags_RowBg | 
                                     ImGuiTableFlags_Resizable | 
                                     ImGuiTableFlags_ScrollY | 
                                     ImGuiTableFlags_ScrollX | 
                                     ImGuiTableFlags_SizingFixedFit;

        if (ImGui::BeginTable("VisualizerTable", totalCols > 0 ? totalCols : 1, tableFlags)) {
            if (hasRowHeaders) {
                ImGui::TableSetupColumn("Row Header", ImGuiTableColumnFlags_WidthFixed, 100.0f);
            }
            for (size_t c = 0; c < columnHeaders.size(); ++c) {
                ImGui::TableSetupColumn(columnHeaders[c].c_str(), ImGuiTableColumnFlags_WidthFixed, 100.0f);
            }
            ImGui::TableHeadersRow();

            for (size_t r = 0; r < cellData.size(); ++r) {
                std::string rowLabel = (r < rowHeaders.size()) ? rowHeaders[r] : "";
                
                // Filter check
                if (filterBuffer[0] != '\0') {
                    bool match = false;
                    if (rowLabel.find(filterBuffer) != std::string::npos) match = true;
                    if (!match) {
                        for (const auto& cell : cellData[r]) {
                            if (cell.find(filterBuffer) != std::string::npos) {
                                match = true;
                                break;
                            }
                        }
                    }
                    if (!match) continue;
                }

                ImGui::TableNextRow();
                int colIdx = 0;

                if (hasRowHeaders) {
                    ImGui::TableSetColumnIndex(colIdx++);
                    ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "%s", rowLabel.c_str());
                }

                for (size_t c = 0; c < cellData[r].size(); ++c) {
                    if (colIdx < totalCols) {
                        ImGui::TableSetColumnIndex(colIdx++);
                        ImGui::TextUnformatted(cellData[r][c].c_str());
                    }
                }
            }

            ImGui::EndTable();
        }
    }

    ImGui::PopStyleVar();
}
