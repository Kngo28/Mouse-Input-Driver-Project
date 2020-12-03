	#
	# mouse behaviors
	#

	# void middle_click_on(void)
	.global middle_click_on
middle_click_on:
	mov $14, %eax
	int $12
	ret

	# void middle_click_off(void)
	.global middle_click_off
middle_click_off:
	mov $15, %eax
	int $12
	ret

	# void right_click_on(void)
	.global right_click_on
right_click_on:
	mov $16, %eax
	int $12
	ret

	# void right_click_off(void)
	.global right_click_off
right_click_off:
	mov $17, %eax
	int $12
	ret

	# void left_click_on(void)
	.global left_click_on
left_click_on:
	mov $18, %eax
	int $12
	ret

	# void left_click_off(void)
	.global left_click_off
left_click_off:
	mov $19, %eax
	int $12
	ret

	# extern void move_mouse(int xm, int ym)
	.global move_mouse
move_mouse:
	mov $20, %eax
	int $12
	ret

	# extern uint32_t show_mouse(void)
	.global show_mouse
show_mouse:
	mov $21, %eax
	int $12
	ret
