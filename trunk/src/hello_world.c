#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>

int main(int argc, char *argv[], char *envp[]) {
	if (argc > 1 && !strcmp(argv[1], "execve")) {
		printf("\nThis is the execve task with PID %d!\n", getpid());
		return 5;
	}
	printf("Hello world (from PID %d)!\n\n", getpid());
	int i;
	printf("The arguments (%i) are:\n", argc);
	for (i = 0; i < argc; i++) {
		printf("\t%i: %s\n", i, argv[i]);
	}
	printf("\nThe environment is:\n");
	i = 0;
        while (envp[i] != NULL)
		printf("\t%s\n", envp[i++]);

	char buffer[512];
	getcwd(buffer, 511);
	printf("Current work directory is \"%s\"\n", buffer);

/*	printf("\nRoot directory:\n");
	DIR *dir = opendir("/");
	struct dirent *entry;
	while (entry = readdir(dir)) {
		printf("\t%s\n", entry->d_name);
	}
	closedir(dir);*/

	printf("\nForking (\"*\" = child, \"-\" = parent)...\n");
	pid_t pid = fork();
	if (!pid) {
		for (i = 0; i < 20; i++) {
			printf("*", i);
			fflush(stdout);
		}
		int res;
		char *exargv[3] = {"/init", "execve", 0};
		res = execve(argv[0], exargv, envp);
		printf("Execve returned %d\n", res);
		return 16;
	}
	printf("Child PID %i\n", pid);
	for (i = 0; i < 20; i++) {
		printf("-", i);
		fflush(stdout);
	}
	int status;
	wait4(pid, &status, 0, NULL);
	printf("\nChild returned with status %i\n", status);
	return 1;
}
