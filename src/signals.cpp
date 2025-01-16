#include "WebServer.hpp"
#include <signal.h>

/*	Global is required to save the exit status when not actually
	exiting a child process, but when pressing ctrl c	*/

int	*g_exit_status;




void	unset_signals(void)
{
	// signal(SIGINT, SIG_IGN);
	// signal(SIGQUIT, SIG_IGN);
	signal(SIGINT, SIG_DFL);
	signal(SIGQUIT, SIG_DFL);
}

void	signals_for_kids(void)
{
	signal(SIGINT, SIG_DFL);
	signal(SIGQUIT, SIG_DFL);
}
