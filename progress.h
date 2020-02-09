#ifndef E2TOOLS_PROGRESS_H
#define E2TOOLS_PROGRESS_H

extern
void init_progress(char *file, struct stat *sbuf);

extern
void update_progress(unsigned long num_bytes);

extern
void finish_progress(void);

#endif /* !defined(E2TOOLS_PROGRESS_H) */
