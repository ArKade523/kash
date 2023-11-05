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

    pid_t left_pid = fork();

    if (left_pid == -1) {
        perror("fork failed");
        return EXIT_FAILURE;
    } else if (left_pid == 0) {
        // In the child process (left side of the pipeline)
        close(pipefd[0]); // Close the unused read end
        dup2(pipefd[1], STDOUT_FILENO); // Redirect stdout to the write end of the pipe
        close(pipefd[1]); // Close the write end after it's duplicated

        int status = left->execute();
        exit(status); // Exit with the status from the left side command
    }

    // Only parent process should reach this code
    pid_t right_pid = fork();

    if (right_pid == -1) {
        perror("fork failed");
        return EXIT_FAILURE;
    } else if (right_pid == 0) {
        // In the child process (right side of the pipeline)
        close(pipefd[1]); // Close the unused write end
        dup2(pipefd[0], STDIN_FILENO); // Redirect stdin to the read end of the pipe
        close(pipefd[0]); // Close the read end after it's duplicated

        int status = right->execute();
        exit(status); // Exit with the status from the right side command
    }

    // Only parent process should reach this code
    close(pipefd[0]); // Parent doesn't use the read end
    close(pipefd[1]); // Parent doesn't use the write end

    int status;
    waitpid(left_pid, &status, 0); // Wait for the left side to finish
    waitpid(right_pid, &status, 0); // Then wait for the right side to finish

    return status; // Return the status of the last command in the pipeline
}

int SequenceNode::execute() {
    int status = left->execute();
    status = right->execute();
    return status;
}