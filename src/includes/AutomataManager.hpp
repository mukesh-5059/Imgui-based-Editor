#pragma once
#include "raylib/raylib.h"
#include "CustomCamera2D.hpp"
#include <vector>
#include <string>
#include <memory>

enum class AutomataType {
    EMPTY,
    DFA,
    NFA,
    INVALID_GRAPH
};

struct TransitionEdge {
    int id;
    int fromNodeId;
    int toNodeId;
    std::string symbols = "a";
};

struct StateNode {
    int id;
    std::string label;
    Vector2 position;
    float radius = 32.0f;
    Color color = { 45, 90, 160, 255 };
    bool isStartState = false;
    bool isAcceptState = false;
};

class AutomataWorkspace {
public:
    int id = 0;
    std::string name;
    
    std::vector<StateNode> nodes;
    std::vector<TransitionEdge> edges;
    int nextNodeId = 0;
    int nextEdgeId = 0;

    int selectedNodeId = -1;
    int selectedEdgeId = -1;

    // Edge Creation State (Hover + Drag from Node)
    bool isCreatingEdge = false;
    int edgeStartNodeId = -1;
    Vector2 edgeTempTargetPos = { 0.0f, 0.0f };

    // Node Drag State
    bool isDraggingNode = false;
    int draggingNodeId = -1;
    Vector2 dragOffset = { 0.0f, 0.0f };

    CustomCamera2D camera2D;
    Vector2 lastClickWorldPos = { 0.0f, 0.0f };
    bool hasClickLocation = false;

    AutomataType currentType = AutomataType::EMPTY;

    AutomataWorkspace(const std::string& name = "Automata Workspace");
    
    StateNode* GetNode(int nodeId);
    TransitionEdge* GetEdge(int edgeId);
    TransitionEdge* FindEdge(int fromId, int toId);
    
    int AddNode(Vector2 pos, const std::string& label = "");
    void DeleteNode(int nodeId);
    
    int AddOrUpdateEdge(int fromId, int toId, const std::string& symbols = "a");
    void DeleteEdge(int edgeId);

    int GetNodeAtPosition(Vector2 worldPos);
    int GetEdgeAtPosition(Vector2 worldPos);

    AutomataType EvaluateGraph();
};

class AutomataManager {
private:
    std::vector<std::shared_ptr<AutomataWorkspace>> workspaces;
    int activeWorkspaceIndex = 0;

public:
    AutomataManager() = default;
    ~AutomataManager() = default;

    static AutomataManager& Get();

    AutomataWorkspace* CreateWorkspace(const std::string& name = "Automata Workspace");
    void RemoveWorkspace(int index);
    
    AutomataWorkspace* GetActiveWorkspace();
    void SetActiveWorkspace(int index) { activeWorkspaceIndex = index; }
    const std::vector<std::shared_ptr<AutomataWorkspace>>& GetWorkspaces() const { return workspaces; }
    int GetActiveWorkspaceIndex() const { return activeWorkspaceIndex; }

    // Transformation Pipelines
    AutomataWorkspace* TriggerSubsetConstruction(AutomataWorkspace* srcWs);
    AutomataWorkspace* TriggerMinimization(AutomataWorkspace* srcWs);
    AutomataWorkspace* TriggerStateElimination(AutomataWorkspace* srcWs);
};
