#include "AST.hpp"
#include "parse_commands.hpp"
#include <sstream>
#include <string>
#include <memory>
#include <iostream>

bool is_builtin_command(const std::string &command) {
    // List of builtin commands
    static const std::vector<std::string> builtin_commands = {
            "cd",
            "pwd",
            "exit"
    };

    for (const auto &builtin_command : builtin_commands) {
        if (command == builtin_command) {
            return true;
        }
    }

    return false;
}

std::unique_ptr<Node> parse_command(const std::string &input) {
    std::istringstream iss(input);
    std::vector<std::string> args;
    std::string token;
    std::unique_ptr<Node> current_node;

     while (iss >> token) {
        if (token == "&&" || token == "||" || token == "|") {
            std::unique_ptr<Node> left_node;
            if (is_builtin_command(args[0])) {
                left_node = std::make_unique<BuiltinCommandNode>(args);
            } else {
                left_node = std::make_unique<CommandNode>(args);
            }
            args.clear(); // clear args for the right side

            std::unique_ptr<Node> right_node = parse_command(std::string(std::istreambuf_iterator<char>(iss), {}));
            
            if (token == "&&") {
                current_node = std::make_unique<AndNode>(std::move(left_node), std::move(right_node));
            } else if (token == "||") {
                current_node = std::make_unique<OrNode>(std::move(left_node), std::move(right_node));
            } else if (token == "|") {
                current_node = std::make_unique<PipelineNode>(std::move(left_node), std::move(right_node));
            }
            break; // Break after setting the right-hand side of the operator
        } else {
            // Not an operator, add to args
            args.push_back(token);
        }
    }

    // If there was no operator, create a command node with the collected args
    if (!current_node) {
        if (is_builtin_command(args[0])) {
            current_node = std::make_unique<BuiltinCommandNode>(args);
        } else {
            current_node = std::make_unique<CommandNode>(args);
        }
    } else if (!args.empty()) {
        // Finalize the current node with the last set of collected arguments
        // This will depend on the type of the current_node
        // If it's an AndNode, OrNode, or PipelineNode, we set the right child
        if (auto andNode = dynamic_cast<AndNode*>(current_node.get())) {
            andNode->setRightChild(std::make_unique<CommandNode>(args));
        } else if (auto orNode = dynamic_cast<OrNode*>(current_node.get())) {
            orNode->setRightChild(std::make_unique<CommandNode>(args));
        } else if (auto pipelineNode = dynamic_cast<PipelineNode*>(current_node.get())) {
            pipelineNode->setRightChild(std::make_unique<CommandNode>(args));
        }
    }

    return current_node;
}