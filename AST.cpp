#include "AST.hpp"
#include <string>
#include <sstream>
#include <memory>
#include <unistd.h>
#include <iostream>

int CommandNode::execute() {
    if (args.empty())
        // Nothing to do
        return EXIT_SUCCESS;

    std::vector<char *> c_args;
    for (const auto &arg : args) {
        c_args.push_back(const_cast<char *>(arg.c_str()));
    }

    // Add a null pointer to the end of the array (required by execvp)
    c_args.push_back(nullptr);

    // Fork a child process
    pid_t pid = fork();

    if (pid == -1) {
        // Error
        perror("fork failed");
        return EXIT_FAILURE;
    } else if (pid == 0) {
        // Child process
        execvp(c_args[0], c_args.data());
        perror("execvp failed");  // execvp doesn't return unless it fails
        exit(EXIT_FAILURE);
    } else {
        // Parent process
        int status;
        waitpid(pid, &status, WUNTRACED);
        return status;
    }
}

int BuiltinCommandNode::execute() {
    if (args[0] == "cd") {
        if (args.size() == 1) {
            // No arguments to cd, go to home directory
            chdir(getenv("HOME"));
        } else if (args.size() == 2) {
            if (chdir(args[1].c_str()) == EXIT_FAILURE) {
                perror("cd failed");
            }
        } else {
            perror("cd: too many arguments");
        }

        return EXIT_SUCCESS;

    } else if (args[0] == "exit") {
        return EXIT_SUCCESS;
    }

    return EXIT_FAILURE;
}

int AndNode::execute() {
    int status = left->execute();
    if (status == EXIT_SUCCESS) {
        status = right->execute();
    }
    return status;
}

int OrNode::execute() {
    int status = left->execute();
    if (status != EXIT_SUCCESS) {
        status = right->execute();
    }
    return status;
}

int PipelineNode::execute() {
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("pipe failed");
        return EXIT_FAILURE;
    }

    pid_t pid = fork();

    if (pid == -1) {
        // Error
        perror("fork failed");
        return EXIT_FAILURE;
    } else if (pid == 0) {
        // Child process
        // Close the read end of the pipe
        close(pipefd[0]);
        // Redirect stdout to the write end of the pipe
        dup2(pipefd[1], STDOUT_FILENO);
        // Close the write end of the pipe
        close(pipefd[1]);
        // Execute the left side of the pipeline
        int status = left->execute();
        exit(status);
    } else {
        // Parent process
        // Close the write end of the pipe
        close(pipefd[1]);
        // Redirect stdin to the read end of the pipe
        dup2(pipefd[0], STDIN_FILENO);
        // Close the read end of the pipe
        close(pipefd[0]);
        // Execute the right side of the pipeline
        int status = right->execute();
        return status;
    }
}