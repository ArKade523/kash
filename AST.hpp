#pragma once
#include <vector>
#include <string>

class Node {
    public:
        virtual ~Node() {}
        virtual int execute() = 0;
};

// A command like ls, cat, etc.
class CommandNode : public Node {
    private:
        std::vector<std::string> args;
    public:
        CommandNode(const std::vector<std::string> &args) : args(args) {}
        virtual int execute() override;

};

// Builtin commands like cd, pwd, and exit
class BuiltinCommandNode : public Node {
    private:
        std::vector<std::string> args;
    public:
        BuiltinCommandNode(const std::vector<std::string> &args) : args(args) {}
        virtual int execute() override;
};

// | operator
class PipelineNode : public Node {
    private:
        std::unique_ptr<Node> left;
        std::unique_ptr<Node> right;
    public:
        PipelineNode(std::unique_ptr<Node> left, std::unique_ptr<Node> right) : left(std::move(left)), right(std::move(right)) {}
        virtual int execute() override;
        void setRightChild(std::unique_ptr<Node> right) {
            this->right = std::move(right);
        }
};

// && operator
class AndNode : public Node {
    private:
        std::unique_ptr<Node> left;
        std::unique_ptr<Node> right;
    public:
        AndNode(std::unique_ptr<Node> left, std::unique_ptr<Node> right) : left(std::move(left)), right(std::move(right)) {}
        virtual int execute() override;
        void setRightChild(std::unique_ptr<Node> right) {
            this->right = std::move(right);
        }
};

// || operator
class OrNode : public Node {
    private:
        std::unique_ptr<Node> left;
        std::unique_ptr<Node> right;
    public:
        OrNode(std::unique_ptr<Node> left, std::unique_ptr<Node> right) : left(std::move(left)), right(std::move(right)) {}
        virtual int execute() override;
        void setRightChild(std::unique_ptr<Node> right) {
            this->right = std::move(right);
        }
};

// ; operator
class SequenceNode : public Node {
    private:
        std::unique_ptr<Node> left;
        std::unique_ptr<Node> right;
    public:
        SequenceNode(std::unique_ptr<Node> left, std::unique_ptr<Node> right) : left(std::move(left)), right(std::move(right)) {}
        virtual int execute() override;
        void setRightChild(std::unique_ptr<Node> right) {
            this->right = std::move(right);
        }
};

// A node that represents a subshell
class SubshellNode : public Node {
    private:
        Node *child;
    public:
        SubshellNode(Node *child) : child(child) {}
        virtual int execute() override;
};

// A node that represents a redirection
class RedirectionNode : public Node {
    private:
        Node *child;
        std::string filename;
        int redirectType; // 0 = <, 1 = >, 2 = >>, 3 = 2>, 4 = 2>>, 5 = &>, 6 = &>>
    public:
        RedirectionNode(Node *child, const std::string &filename, int redirectType) : 
            child(child), filename(filename), redirectType(redirectType) {}
        virtual int execute() override;
};

// A node that represents a background process
class BackgroundNode : public Node {
    private:
        Node *child;
    public:
        BackgroundNode(Node *child) : child(child) {}
        virtual int execute() override;
};

// A node that represents a negation
class NegateNode : public Node {
    private:
        Node *child;
    public:
        NegateNode(Node *child) : child(child) {}
        virtual int execute() override;
};

// A node that represents a variable assignment
class AssignmentNode : public Node {
    private:
        std::string var;
        std::string val;
    public:
        AssignmentNode(const std::string &var, const std::string &val) : var(var), val(val) {}
        virtual int execute() override;
};

// A node that represents a command substitution
class CommandSubstitutionNode : public Node {
    private:
        Node *child;
    public:
        CommandSubstitutionNode(Node *child) : child(child) {}
        virtual int execute() override;
};
