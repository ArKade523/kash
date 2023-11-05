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

// Handler for SIGINT that doesn't do anything.
// This is set only for child processes so that they can be interrupted
// but the shell itself ignores the interrupt.
void SIGINT_handler_child(int signum) {}

bool is_builtin_command(const std::string &command) {
    // List of builtin commands
    static const std::vector<std::string> builtin_commands = {
            "cd",
            "exit"
    };

    for (const auto &builtin_command : builtin_commands) {
        if (command == builtin_command) {
            return true;
        }
    }

    return false;
}

int execute_builtin_command(const std::vector<std::string> &args) {
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

    } else if (args[0] == "exit") {
        return EXIT_SUCCESS;
    }

    return EXIT_FAILURE;
}

std::vector<std::string> parse_command(const std::string &input) {
    std::istringstream iss(input);
    std::vector<std::string> args;
    std::string arg;

    while (iss >> arg) {
        args.push_back(arg);
    }

    return args;
}

void execute_command(const std::vector<std::string> &args) {
    if (args.empty())
        // Nothing to do
        return;
    
    if (is_builtin_command(args[0])) {
        execute_builtin_command(args);
        return;
    }

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
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        // In the child process
        execvp(c_args[0], c_args.data());

        // Reset signal handling to default behavior for the child process
        signal(SIGINT, SIGINT_handler_child);

        // If execvp returns, it must have failed
        perror("execvp failed");
        exit(EXIT_FAILURE);
    } else {
        // In the parent process, waiting for the child process to finish
        // Wait for the command to finish before displaying a new prompt
        command_running = 1;
        int status;
        waitpid(pid, &status, WUNTRACED);
        command_running = 0;
    }
}

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

int main() {
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
        if (!command_running) {
            input = readline(prompt.c_str());
        } else {
            // Reset the flag
            command_running = 0;
            input = NULL;
            // Continue to the next iteration, skipping over the readline call
            continue;
        }


        if (input == nullptr) {
            std::cout << "exit" << std::endl;
            // EOF received (Ctrl+O)
            break;
        }

        // Do nothing if the command is empty
        if (std::string(input).empty()) {
            free(input);
            continue;
        }

        // Add the command to the history
        add_history(input);

        // Convert input to std::string, then free the input
        std::string input_str(input);
        free(input);

        // Exit the shell on 'exit' command
        if (input_str == "exit") {
            break;
        }

        std::vector<std::string> args = parse_command(input_str);

        if (!args.empty()) {
            execute_command(args);
        }

    }

    // Write history to file
    write_history(history_path.c_str());

    // Clean up readline history
    clear_history();

    return EXIT_SUCCESS;
}