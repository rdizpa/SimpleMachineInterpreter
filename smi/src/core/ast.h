#ifndef _AST_H_
#define _AST_H_

#include <iostream>
#include <string>
#include <vector>

enum class NodeType {
    PROGRAM,
    LABEL,
    INSTR,
    IDENTIFIER,
    ASSIGN
};

class Node {
   private:
    NodeType m_type;
    int m_line;
    int m_index;
    int m_column;

   public:
    Node(NodeType type, int index, int line, int column)
        : m_type(type), m_index(index), m_line(line), m_column(column) {}

    NodeType type() const { return m_type; }
    bool typeMatch(NodeType type) const { return m_type == type; }

    int index() const { return m_index; }
    int line() const { return m_line; }
    int column() const { return m_column; }

    virtual Node* clone() const { return nullptr; }
    virtual bool operator==(const Node& other) { return false; }
    bool operator!=(const Node& other) { return !(*this == other); }

    virtual ~Node() {
        // std::cout << "Destroyed node of type " << (int)m_type << std::endl;
    }
};

class Program : public Node {
   private:
    std::vector<Node*> m_body;

   public:
    Program(std::vector<Node*> body, int index, int line, int column)
        : Node(NodeType::PROGRAM, index, line, column), m_body(body) {}

    std::vector<Node*>& getBody() { return m_body; }

    ~Program() {
        for (Node* node : m_body) {
            delete node;
        }
    }
};

class Label : public Node {
   private:
    std::string m_name;

   public:
    Label(std::string name, int index, int line, int column)
        : Node(NodeType::LABEL, index, line, column), m_name(name) {}

    std::string getName() const { return m_name; }

    Node* clone() const { return new Label(m_name, index(), line(), column()); }

    bool operator==(const Node& other) {
        return other.typeMatch(NodeType::LABEL) && static_cast<const Label*>(&other)->m_name == m_name;
    }

    ~Label() = default;
};

class Instruction : public Node {
   private:
    std::string m_instr;
    Node *m_op1, *m_op2;

   public:
    Instruction(std::string instr, Node* op1, Node* op2, int index, int line, int column)
        : Node(NodeType::INSTR, index, line, column), m_instr(instr), m_op1(op1), m_op2(op2) {}

    std::string getInstr() const { return m_instr; }
    Node* getOp1() const { return m_op1; }
    Node* getOp2() const { return m_op2; }

    Node* clone() const {
        return new Instruction(m_instr, m_op1 ? m_op1->clone() : nullptr, m_op2 ? m_op2->clone() : nullptr, index(),
                               line(), column());
    }

    bool operator==(const Node& other) {
        if (!other.typeMatch(NodeType::INSTR)) return false;

        const Instruction* inst = static_cast<const Instruction*>(&other);

        if (inst->m_instr != m_instr) return false;

        if (*inst->m_op1 != *m_op1) return false;

        if (!inst->m_op2 && m_op2 || inst->m_op2 && !m_op2) return false;
        if (m_op2 && *inst->m_op2 != *m_op2) return false;

        return true;
    }

    ~Instruction() {
        delete m_op1;
        delete m_op2;
    }
};

class Identifier : public Node {
   private:
    std::string m_value;

   public:
    Identifier(std::string value, int index, int line, int column)
        : Node(NodeType::IDENTIFIER, index, line, column), m_value(value) {}

    std::string getValue() const { return m_value; }

    Node* clone() const { return new Identifier(m_value, index(), line(), column()); }

    bool operator==(const Node& other) {
        return other.typeMatch(NodeType::IDENTIFIER) && static_cast<const Identifier*>(&other)->m_value == m_value;
    }

    ~Identifier() = default;
};

class Assignment : public Node {
   private:
    std::string m_name;
    std::string m_value;

   public:
    Assignment(std::string name, std::string value, int index, int line, int column)
        : Node(NodeType::ASSIGN, index, line, column), m_name(name), m_value(value) {}

    std::string getName() { return m_name; }
    std::string getValue() { return m_value; }

    Node* clone() const { return new Assignment(m_name, m_value, index(), line(), column()); }

    bool operator==(const Node& other) {
        return other.typeMatch(NodeType::ASSIGN) && static_cast<const Assignment*>(&other)->m_name == m_name &&
               static_cast<const Assignment*>(&other)->m_value == m_value;
    }

    ~Assignment() = default;
};

#endif
