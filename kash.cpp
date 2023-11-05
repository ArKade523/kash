#include "AST.hpp"
#include "parse_commands.hpp"
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <unistd.h>
#include <sys/wait.h>
#include <cstring>
#include <signal.h>
#include <readline/readline.h>
#include <readline/history.h>

const std::string history_path = std::string(getenv("HOME")) + "/.kash_history";
volatile sig_atomic_t command_running = 0;

// Signal handler for SIGINT
void sigint_handler(int signal_num) {
    // Main shell process, handle by printing a newline
    std::cout << std::endl;
    if (!command_running) {
        // Only redisplay the prompt if no command is currently running
        rl_on_new_line();
        rl_replace_line("", 0);
        rl_redisplay();
    }
}

std::string get_prompt_path() {
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != nullptr) {
        std::string current_dir(cwd);
        const char* home_dir = getenv("HOME");

        if (home_dir != NULL) {
            std::string home(home_dir);

            // check if the current directory is a subdirectory of the home directory
            if (current_dir.find(home) == 0) {
                // replace the home directory with ~
                current_dir.replace(0, home.length(), "~");
            }
        }

        return current_dir;
    } else {
        perror("getcwd failed");
        return "";
    }

    return "";
}

int main(void) {
    char* input;
    std::string prompt = "kash: " + get_prompt_path() + " > ";

    // Initialize readline history
    using_history();

    // Read history from file
    if (read_history(history_path.c_str()) != EXIT_SUCCESS)
        perror("read_history failed");

    // Register SIGINT handler
    struct sigaction sigint_action_shell, sigint_action_default;
    sigint_action_shell.sa_handler = sigint_handler; // Set the handler function for the shell
    sigint_action_default.sa_handler = SIG_DFL; // Default action for child processes

    // Block all signals during the handler
    sigfillset(&sigint_action_shell.sa_mask);
    sigfillset(&sigint_action_default.sa_mask);

    sigint_action_shell.sa_flags = 0;
    sigint_action_default.sa_flags = 0;

    if (sigaction(SIGINT, &sigint_action_shell, nullptr) != EXIT_SUCCESS) {
        perror("sigaction failed");
        return EXIT_FAILURE;
    }

    while(1) {
        // Update prompt before reading input
        prompt = "kash: " + get_prompt_path() + " > ";
        
        // Read input
        input = readline(prompt.c_str());


        if (input == nullptr) {
            // EOF reached, exit
            break;
        }
        
        std::string input_str = std::string(input);

        // Exit the shell on 'exit' command
        if (input_str == "exit") {
            break;
        }

        if (strlen(input) != 0) {
            // Add to history
            add_history(input);

            // Write history to file
            if (write_history(history_path.c_str()) != EXIT_SUCCESS)
                perror("write_history failed");

            // Parse input
            std::unique_ptr<Node> root = parse_command(input);


            // Execute the command
            command_running = 1;
            int status = root->execute();
            command_running = 0;

            if (status != EXIT_SUCCESS) {
                std::cout << "Command failed with status " << status << std::endl;
            }
        }

        // Free input
        free(input);
    }

    // Write history to file
    write_history(history_path.c_str());

    // Clean up readline history
    clear_history();

    return EXIT_SUCCESS;
}