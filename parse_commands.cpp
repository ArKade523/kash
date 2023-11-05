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
        if (token == "&&") {
            // && operator found, create an AND node and update the current node
            std::unique_ptr<Node> left_node = std::make_unique<CommandNode>(args);
            args.clear(); // clear args for the right side
            // Using a ternary operator here to check if we have a current node
            current_node = current_node
                ? std::make_unique<AndNode>(std::move(current_node), std::move(left_node))
                : std::move(left_node);
        } else if (token == "||") {
            // || operator found, create an OR node and update the current node
            std::unique_ptr<Node> left_node = std::make_unique<CommandNode>(args);
            args.clear(); // clear args for the right side
            // Using a ternary operator here to check if we have a current node
            current_node = current_node
                ? std::make_unique<OrNode>(std::move(current_node), std::move(left_node))
                : std::move(left_node);
        } else if (token == "|") {
            // | operator found, create a PIPELINE node and update the current node
            std::unique_ptr<Node> left_node = std::make_unique<CommandNode>(args);
            args.clear(); // clear args for the right side
            // Using a ternary operator here to check if we have a current node
            current_node = current_node
                ? std::make_unique<PipelineNode>(std::move(current_node), std::move(left_node))
                : std::move(left_node);
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