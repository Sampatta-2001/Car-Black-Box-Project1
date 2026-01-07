#ifndef CAR_BLACK_BOX_H
#define CAR_BLACK_BOX_H

void display_dashboard(unsigned char event[], unsigned char speed);
void log_event(unsigned char event[],unsigned char speed);
unsigned char login(unsigned char key,unsigned char reset_flag);
unsigned char menu_screen(unsigned char key,unsigned char reset_flag);
unsigned char view_log_screen(unsigned char key,unsigned char reset_flag);
unsigned char clear_log_screen(unsigned char reset_flag,unsigned char speed);
unsigned char download_log_screen(unsigned char key, unsigned char reset_flag);
unsigned char set_time_log_screen(unsigned char key,unsigned char reset_flag,unsigned char speed);
unsigned char change_password_screen(unsigned char key, unsigned char reset_flag,unsigned char speed);
void clear_screen();
#endif	/* CAR_BLACK_BOX_H */