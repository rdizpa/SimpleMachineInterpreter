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

    ~Assignment() = default;
};

#endif
