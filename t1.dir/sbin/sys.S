	#
	# user-side system calls
	#
	# System calls use a special convention:
        #     %eax  -  system call number
        #
        #

	# void exit(int status)
	.global exit
exit:
	mov $0,%eax
	int $48
	ret

	# ssize_t write(int fd, void* buf, size_t nbyte)
	.global write
write:
	mov $1,%eax
	int $48
	ret

	# int fork()
	.global fork
fork:
	push %ebx
	push %esi
	push %edi
	push %ebp
	mov $2,%eax
	int $48
	pop %ebp
	pop %edi
	pop %esi
	pop %ebx
	ret

	# int sem(uint32_t init)
	.global sem
sem:
	mov $3,%eax
	int $48
	ret

	# int up(int s)
	.global up
up:
	mov $4,%eax
	int $48
	ret

	# int down(int s)
	.global down
down:
	mov $5,%eax
	int $48
	ret

	# int close(int id)
	.global close
close:
	mov $6,%eax
	int $48
	ret

	# int shutdown(void)
	.global shutdown
shutdown:
	mov $7,%eax
	int $48
	ret

	# int wait(int id, uint32_t *ptr)
	.global wait
wait:
	mov $8,%eax
	int $48
	ret

	# int execl(const char* path, const char* arg0, ....);
	.global execl
execl:
	mov $9,%eax
	int $48
	ret

	# int open(const char* fn)
	.global open
open:
	mov $10,%eax
	int $48
	ret

	# ssize_t len(int fd)
	.global len
len:
	mov $11,%eax
	int $48
	ret

	# ssize_t read(int fd, void* buffer, size_t n)
	.global read
read:
	mov $12,%eax
	int $48
	ret

	# off_t seek(int fd, off_t off)
	.global seek
seek:
	mov $13,%eax
	int $48
	ret



	#
	# mouse behaviors
	#

	# void middle_click_on(void)
	.global middle_click_on
middle_click_on:
	mov $0, %eax
	int $12
	ret

	# void middle_click_off(void)
	.global middle_click_off
middle_click_off:
	mov $1, %eax
	int $12
	ret

	# void right_click_on(void)
	.global right_click_on
right_click_on:
	mov $2, %eax
	int $12
	ret

	# void right_click_off(void)
	.global right_click_off
right_click_off:
	mov $3, %eax
	int $12
	ret

	# void left_click_on(void)
	.global left_click_on
left_click_on:
	mov $4, %eax
	int $12
	ret

	# void left_click_off(void)
	.global left_click_off
left_click_off:
	mov $5, %eax
	int $12
	ret

	# extern void move_mouse(int xm, int ym)
	.global move_mouse
move_mouse:
	mov $6, %eax
	int $12
	ret

	# extern uint32_t show_mouse(void)
	.global show_mouse
show_mouse:
	mov $7, %eax
	int $12
	ret
