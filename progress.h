#ifndef E2TOOLS_PROGRESS_H
#define E2TOOLS_PROGRESS_H

void init_progress(char *file, struct stat *sbuf);
void update_progress(unsigned long num_bytes);
void finish_progress();

#endif /* !E2TOOLS_PROGRESS_H */
