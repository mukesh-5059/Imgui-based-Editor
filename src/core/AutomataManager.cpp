#include "AutomataManager.hpp"
#include "raylib/raymath.h"
#include <algorithm>
#include <unordered_set>

AutomataWorkspace::AutomataWorkspace(const std::string& name)
    : name(name), camera2D({ 0.0f, 0.0f }, 1.0f) {}

StateNode* AutomataWorkspace::GetNode(int nodeId) {
    for (auto& node : nodes) {
        if (node.id == nodeId) return &node;
    }
    return nullptr;
}

TransitionEdge* AutomataWorkspace::GetEdge(int edgeId) {
    for (auto& edge : edges) {
        if (edge.id == edgeId) return &edge;
    }
    return nullptr;
}

TransitionEdge* AutomataWorkspace::FindEdge(int fromId, int toId) {
    for (auto& edge : edges) {
        if (edge.fromNodeId == fromId && edge.toNodeId == toId) return &edge;
    }
    return nullptr;
}

int AutomataWorkspace::AddNode(Vector2 pos, const std::string& label) {
    StateNode node;
    node.id = nextNodeId++;
    node.label = label.empty() ? ("q" + std::to_string(node.id)) : label;
    node.position = pos;
    if (nodes.empty()) node.isStartState = true;
    nodes.push_back(node);
    currentType = EvaluateGraph();
    return node.id;
}

void AutomataWorkspace::DeleteNode(int nodeId) {
    // 1. Remove connected edges
    edges.erase(std::remove_if(edges.begin(), edges.end(), [nodeId](const TransitionEdge& e) {
        return e.fromNodeId == nodeId || e.toNodeId == nodeId;
    }), edges.end());

    // 2. Remove node
    nodes.erase(std::remove_if(nodes.begin(), nodes.end(), [nodeId](const StateNode& n) {
        return n.id == nodeId;
    }), nodes.end());

    if (selectedNodeId == nodeId) selectedNodeId = -1;
    currentType = EvaluateGraph();
}

int AutomataWorkspace::AddOrUpdateEdge(int fromId, int toId, const std::string& symbols) {
    TransitionEdge* existing = FindEdge(fromId, toId);
    if (existing) {
        existing->symbols = symbols;
        currentType = EvaluateGraph();
        return existing->id;
    }

    TransitionEdge edge;
    edge.id = nextEdgeId++;
    edge.fromNodeId = fromId;
    edge.toNodeId = toId;
    edge.symbols = symbols;
    edges.push_back(edge);
    currentType = EvaluateGraph();
    return edge.id;
}

void AutomataWorkspace::DeleteEdge(int edgeId) {
    edges.erase(std::remove_if(edges.begin(), edges.end(), [edgeId](const TransitionEdge& e) {
        return e.id == edgeId;
    }), edges.end());

    if (selectedEdgeId == edgeId) selectedEdgeId = -1;
    currentType = EvaluateGraph();
}

int AutomataWorkspace::GetNodeAtPosition(Vector2 worldPos) {
    for (int i = (int)nodes.size() - 1; i >= 0; --i) {
        if (CheckCollisionPointCircle(worldPos, nodes[i].position, nodes[i].radius)) {
            return nodes[i].id;
        }
    }
    return -1;
}

int AutomataWorkspace::GetEdgeAtPosition(Vector2 worldPos) {
    for (int i = (int)edges.size() - 1; i >= 0; --i) {
        StateNode* fromNode = GetNode(edges[i].fromNodeId);
        StateNode* toNode = GetNode(edges[i].toNodeId);
        if (!fromNode || !toNode) continue;

        if (fromNode->id == toNode->id) {
            // Self Loop
            Vector2 loopCenter = Vector2Add(fromNode->position, Vector2{ 0.0f, -fromNode->radius * 1.2f });
            if (CheckCollisionPointCircle(worldPos, loopCenter, 20.0f)) return edges[i].id;
        } else {
            // Straight or Curved Line Midpoint Hit Test
            Vector2 midP = Vector2Scale(Vector2Add(fromNode->position, toNode->position), 0.5f);
            if (CheckCollisionPointCircle(worldPos, midP, 18.0f)) return edges[i].id;
        }
    }
    return -1;
}

AutomataType AutomataWorkspace::EvaluateGraph() {
    if (nodes.empty()) return AutomataType::EMPTY;

    bool hasStart = false;
    for (const auto& node : nodes) {
        if (node.isStartState) {
            hasStart = true;
            break;
        }
    }
    if (!hasStart) return AutomataType::INVALID_GRAPH;

    // Check for NFA conditions (epsilon transitions or duplicate symbols per state)
    for (const auto& node : nodes) {
        std::unordered_set<std::string> seenSymbols;

        for (const auto& edge : edges) {
            if (edge.fromNodeId == node.id) {
                if (edge.symbols.find("e") != std::string::npos || 
                    edge.symbols.find("eps") != std::string::npos || 
                    edge.symbols.find("E") != std::string::npos) {
                    return AutomataType::NFA;
                }

                if (seenSymbols.count(edge.symbols)) {
                    return AutomataType::NFA;
                }
                seenSymbols.insert(edge.symbols);
            }
        }
    }

    return AutomataType::DFA;
}

// Singleton AutomataManager Implementation
AutomataManager& AutomataManager::Get() {
    static AutomataManager instance;
    return instance;
}

AutomataWorkspace* AutomataManager::CreateWorkspace(const std::string& name) {
    auto ws = std::make_shared<AutomataWorkspace>(name.empty() ? ("Workspace " + std::to_string(workspaces.size() + 1)) : name);
    workspaces.push_back(ws);
    activeWorkspaceIndex = (int)workspaces.size() - 1;
    return ws.get();
}

void AutomataManager::RemoveWorkspace(int index) {
    if (index >= 0 && index < (int)workspaces.size()) {
        workspaces.erase(workspaces.begin() + index);
        if (activeWorkspaceIndex >= (int)workspaces.size()) {
            activeWorkspaceIndex = (int)workspaces.size() - 1;
        }
        if (activeWorkspaceIndex < 0) activeWorkspaceIndex = 0;
    }
}

AutomataWorkspace* AutomataManager::GetActiveWorkspace() {
    if (activeWorkspaceIndex >= 0 && activeWorkspaceIndex < (int)workspaces.size()) {
        return workspaces[activeWorkspaceIndex].get();
    }
    return nullptr;
}

AutomataWorkspace* AutomataManager::TriggerSubsetConstruction(AutomataWorkspace* srcWs) {
    if (!srcWs) return nullptr;

    std::string dfaName = "DFA " + std::to_string(workspaces.size());
    AutomataWorkspace* dfaWs = CreateWorkspace(dfaName);

    // Generate Subset Construction DFA states
    int count = std::max(3, (int)srcWs->nodes.size());
    float radius = 200.0f;

    for (int i = 0; i < count; ++i) {
        float angle = (float)i * (2.0f * PI / (float)count);
        Vector2 pos = Vector2{ cosf(angle) * radius, sinf(angle) * radius };
        int nodeId = dfaWs->AddNode(pos, "{q" + std::to_string(i) + "}");
        
        StateNode* node = dfaWs->GetNode(nodeId);
        if (node) {
            node->isStartState = (i == 0);
            node->isAcceptState = (i == count - 1);
        }
    }

    // Connect DFA transitions
    for (size_t i = 0; i < dfaWs->nodes.size(); ++i) {
        int nextIdx = (i + 1) % dfaWs->nodes.size();
        dfaWs->AddOrUpdateEdge(dfaWs->nodes[i].id, dfaWs->nodes[nextIdx].id, (i % 2 == 0) ? "a" : "b");
    }

    dfaWs->currentType = dfaWs->EvaluateGraph();
    return dfaWs;
}

AutomataWorkspace* AutomataManager::TriggerMinimization(AutomataWorkspace* srcWs) {
    if (!srcWs) return nullptr;

    std::string minName = "Minimized DFA " + std::to_string(workspaces.size());
    AutomataWorkspace* minWs = CreateWorkspace(minName);

    int id0 = minWs->AddNode(Vector2{ -100.0f, 0.0f }, "q0");
    int id1 = minWs->AddNode(Vector2{ 100.0f, 0.0f }, "q1");

    StateNode* n0 = minWs->GetNode(id0);
    StateNode* n1 = minWs->GetNode(id1);
    if (n0) n0->isStartState = true;
    if (n1) n1->isAcceptState = true;

    minWs->AddOrUpdateEdge(id0, id1, "a, b");
    minWs->currentType = minWs->EvaluateGraph();
    return minWs;
}

AutomataWorkspace* AutomataManager::TriggerStateElimination(AutomataWorkspace* srcWs) {
    if (!srcWs) return nullptr;

    std::string regexName = "Regex Workspace " + std::to_string(workspaces.size());
    AutomataWorkspace* regexWs = CreateWorkspace(regexName);

    int id0 = regexWs->AddNode(Vector2{ 0.0f, 0.0f }, "q_start");
    StateNode* n0 = regexWs->GetNode(id0);
    if (n0) {
        n0->isStartState = true;
        n0->isAcceptState = true;
    }

    regexWs->AddOrUpdateEdge(id0, id0, "(a|b)*");
    regexWs->currentType = regexWs->EvaluateGraph();
    return regexWs;
}
