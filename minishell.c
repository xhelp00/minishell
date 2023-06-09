/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   minishell.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: phelebra <phelebra@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/30 15:56:26 by x230              #+#    #+#             */
/*   Updated: 2023/06/27 16:11:21 by phelebra         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

#define LINE_SIZE 1024
#define PATH_SEP ":"
#define EXEC_ERROR 42

// main loop to run the shell
void    shell_loop(void)
{
	t_parsed	*head;
    char    	*line;
	char		**ebloid;
    int     	status;
	extern char **environ;	// our only global variable ?

	status = 1;
	while (status)
	{
		line = readline("\033[30m\033[101;5;7m⛧minihell⛧\033[0m\033[31m>\033[0m ");
        if (!line) // if readline returns NULL, it means we hit an EOF (like Ctrl+D)
            break;
		if (line && *line)
            add_history(line);
		// test_tokenize(line, __environ);
		ebloid = lexer(line, environ);
		head = fill_list(ebloid);
		// test_parser(head);
		check_opts(head, environ);
		free(line);
	}
}

// check for pipes or redirects
int	check_opts(t_parsed *cmd, char **envp)
{
	// if (cmd->op == PIPE)
	// 	execute_pipe(cmd, envp);
	// else if (cmd->op == RED_IN)
	// 	execute_red_in(cmd, envp);
	// else if (cmd->op == RED_OUT)
	// 	execute_red_out(cmd, envp)
	// else if (cmd->op == RED_APP)
	// 	execute_red_app(cmd, envp);
	// else if (cmd->op == HEREDOC)
	// 	execute_heredoc(cmd, envp);
	// else
		execute(cmd, envp);
	return (1);
}

int	execute(t_parsed *cmd, char **envp)
{
	char	*path;
	pid_t	pid;
	int		status;

	if (check_builtins(cmd->args, envp)) // check for builtins first
        return (1);
	path = get_path(cmd->args[0], envp);
	pid = fork();
	if (pid == 0)
	{
		// child process
		if (execve(path, cmd->args, envp) == -1)
		{
			free(path);
			exit(EXEC_ERROR);
		}
	}
	else if (pid < 0)
		you_fucked_up("Fork failed");
	else
	{
		// parent process
		waitpid(pid, &status, WUNTRACED);
		if (WIFEXITED(status) && WEXITSTATUS(status) == EXEC_ERROR)
			ft_printf("Command not found: %s \n", cmd->args[0]);
	}
	free(path);
	return (1);
}


// Gets PATH environment variable and saves into path_env
// Loops through each directory in path_env separated by PATH_SEP
// Uses get_path_token function to get full path, returns it
char	*get_path(char *cmd, char **envp)
{
	int		i;
	char	*path_env = NULL;
	char	*path;
	int		cmd_len;

	i = 0;
	cmd_len = ft_strlen(cmd);
	while (envp[i] != NULL)
	{
		if (ft_strncmp(envp[i], "PATH=", 5) == 0)
		{
			path_env = ft_strdup(envp[i] + 5);
			break ;
		}
		i++;
	}
	if (path_env == NULL) // if PATH= was not found in envp
		return NULL;
	path = get_path_token(cmd, path_env, cmd_len);
	return (path);
}

// Searches through each path in the PATH environment variable
// Construct the full path to the command
// Uses access() to check if path is executable
// Returns it if it is
char	*get_path_token(char *cmd, char *path_env, int cmd_len)
{
	char	*path_token;
	char	*path;

	path_token = ft_strtok(path_env, PATH_SEP);
	while (path_token != NULL)
	{
		path = ft_calloc(ft_strlen(path_token) + cmd_len + 2, 1);
		ft_strcat(path, path_token);
		ft_strcat(path, "/");
		ft_strcat(path, cmd);
		if (access (path, X_OK) == 0)
		{
			free(path_env);
			return (path);
		}
		free(path);
		path_token = ft_strtok(NULL, PATH_SEP);
	}
	free(path_env);
	return (NULL);
}
