
# mp1.S - life-or-death version
# Good luck, and have fun!

.data
	
    # Constants for accessing the fields of a struct keystroke_args
    # struct keystroke_args is defined in rtc.h and mp1.h

	KA_SIZE	  = 14   # 14 bytes when copying
	KA_STACK  = 16   # use 16 bytes on stack for alignment
	DIRECTION = 0
	SELECTOR  = 2
	GUESS     = 4
	HINT      = 9

    # Constants for accessing the fields of a struct game_status
    # struct game_status is defined in rtc.h and mp1.h

	GS_SIZE   = 8	# 8 bytes
	POP 	  = 0
	INF	  = 4

    # Constants for screen size

    	SCR_WIDTH  = 80
	SCR_HEIGHT = 20
	SCR_SIZE   = ((SCR_WIDTH) * (SCR_HEIGHT))

    # ASCII cell images

        CELL_LIVE = 0x78
	CELL_DEAD = 0x20


# Data shared between this file and rtc.c helper functions

# This '.globl' directive makes these symbols visible externally
.globl rand_seed, aggression

			.align 8
rand_seed:		.quad 0x0	# Pseudo-random number seed
aggression:		.long 0x0	# Aggression level of virus


# Data private to this file

current_board:		.long 0x0	# Address of current board
next_board:		.long 0x0	# Address of next board
population:		.long 0x0	# Remaining human population
infection:		.long 0x0	# Count of live virus cells
select_image:		.string "==>"	# Picture of the selector
select_blank:		.string "   "	# Picture to overwrite selector
base_array:             .string "ACGT"  # Array of characters for bases


.text

# void mp1_poke(void);
# You can use this function to write to video memory.
#
# Interface: Register-based arguments (not C-style)
#    Inputs: %cl  - The byte you wish to write
#            %eax - Offset from the start of video memory that you wish
#                   to write to
#   Outputs: Text-mode video screen is written to at location %eax with
#            the byte in %cl
# Registers: Clobbers EDX
 # now it can change edx
mp1_poke:
		pushl	%edx
		movl    vmem_base_addr(,1),%edx
		movb    %cl,(%edx,%eax,1)
		popl	%edx
		ret

#   ----------------- Exported functions ---------------------

# void mp1_rtc_tasklet(unsigned long ignore);
# Performs three tasks:
#	(1) updates the life gamnge board and calculates infection count
#	(2) swaps the two boards
#	(3) applies population reduction based on infection count
#	(4) redraws the screen
#	(5) notifies the user code by calling mp1_notify_user
# Inputs   : none
# Outputs  : none
# Registers: Standard C calling convention

.globl mp1_rtc_tasklet
mp1_rtc_tasklet:
	pushl	%ebp	# save old frame pointer
	movl 	%esp,%ebp	# point to new frame
	pushl	%ebx
	pushl	%esi	
	pushl	%edi

	movl	$0,infection
# Begin by checking the value of current board. If it is 0, your tasklet should return immediately. 
	movl	current_board, %ecx	# ecx<-M[label]
	movl	next_board,%ebx
	cmpl	$0, %ecx
	je		end_tasklet	# IF it's 0, return
/* First,  use the current board to update the next board. 
 * For every cell in the current board that is not on any of the four boundaries, 
 * determine whether the corresponding cell in the next board should be live or dead 
	*/
	# now %ecx is a pointer to the start of current board
	addl	$81,%ecx	
	addl	$81,%ebx	# change the pointer to row1 col1
	movl	$18,%edi	# outer loop
		
out_loop:
	movl	$78,%esi
inner_loop:

	pushl	%ecx
	pushl	%edx
	pushl	%ecx	# pointer to the %ecx
	call	neighbor_count	# return the 2nd arg of tick_result
	addl	$4,%esp
	popl	%edx
	popl	%ecx

	movl	(%ecx),%edx	# current board status
	pushl 	%ecx
	pushl	%edx

	pushl	%eax
	pushl	%edx 
	call	tick_result		# return the next board's status
	addl	$8,%esp
	popl	%edx
	popl	%ecx

	movl	%eax,(%ebx)
	imull	$10,%eax,%eax
	addl	%eax,infection	# update infection
	incl	%ecx
	incl	%ebx
	decl	%esi
	cmpl	$0,%esi
	ja		inner_loop
	addl	$2,%ebx
	addl	$2,%ecx
	decl	%edi
	cmpl	$0,%edi
	ja		out_loop

# The second step for your tasklet is to swap the two board pointers (to implement double-buffering).
	movl	current_board,%ebx	# point to current board
	movl	next_board,%ecx	# point to next board
	movl	%ebx,next_board
	movl	%ecx,current_board
# Next, subtract the new infection value from the population; 
# treat these values as unsigned, and be sure not to let population drop below 0 (set it to 0 if it does).
	movl	population,%ebx
	movl	infection,%ecx
	subl	%ecx,%ebx
	cmpl	$0,%ebx
	jge 	keep_population	# if population > infection
	movl	$0,%ebx	# set to 0
keep_population:
	movl	%ebx,population	# should we update?
	pushl	%eax
	pushl	%ecx
	pushl	%edx
	call	draw_screen
	popl	%edx
	popl	%ecx
	popl	%eax
# Finally, your tasklet must notify the user-space program by calling mp1 notify user before the tasklet returns:
# void mp1 notify user (void);
	pushl	%eax
	pushl	%ecx
	pushl	%edx
	call	mp1_notify_user
	popl	%edx
	popl	%ecx
	popl	%eax

end_tasklet:
	popl 	%edi
	popl	%esi
	popl	%ebx
	leave	
		ret


# int mp1_ioctl(unsigned long arg, unsigned int cmd)
# The dispatch function for the MP1 ioctls - should use the cmd argument
# and a jumptable to execute one of the specific ioctls implemented below.
# Inputs   : unsigned long arg - parameter to the mp1_ioctl_....
#          : unsigned int cmd  - specifies which mp1_ioctl_... function 
#          :                     to execute
# Outputs  : Returns an integer - depends on which ioctl() function is called
# Registers: Standard C calling convention

.globl mp1_ioctl
mp1_ioctl:		# huge bug here.  don't push ebp !!!
/*	pushl	%ebp	# save old frame pointer
	movl 	%esp,%ebp	# point to new frame
	pushl	%ebx
	pushl	%esi	
	pushl	%edi
	*/

	movl	8(%esp),%ecx	# ecx is cmd
	cmpl	$4, %ecx
	ja		default
	jmp		*jump_table(,%ecx,4)

ioctl_end:
/*	popl 	%edi
	popl	%esi
	popl	%ebxbzlinuser
	leave	
*/
		ret
default:
	movl	$-1, %eax
	jmp		ioctl_end 

jump_table:
	.long	mp1_ioctl_startgame, mp1_ioctl_endgame,mp1_ioctl_keystroke, mp1_ioctl_getstatus, mp1_ioctl_vaccinate

# ----------------- Functions private to this file -------------------

	# draw the whole screen after every call
	# callee saved
draw_screen:	
	# For simplicity, your implementation may assume a SCR_WIDTH of 80.
	pushl	%ebp	# save old frame pointer
	movl 	%esp,%ebp	# point to new frame
	pushl	%ebx
	pushl	%esi	
	pushl	%edi

	movl	$0, %edi		# offset
	movl	current_board,%ebx
loop_draw_screen:
	movb	(%ebx),%cl	# 0 or 1
	cmpb	$0,%cl
	ja		is_alive
	movb	$CELL_DEAD, %cl
	imull	$2,%edi,%eax	# eax = edi * 2
	call	mp1_poke		
	incl	%ebx
	incl	%edi
	cmpl	$1600,%edi
	je		finish_draw
	jmp		loop_draw_screen
is_alive:
	movb	$CELL_LIVE, %cl
	imull	$2,%edi,%eax	# eax = edi * 2
	call	mp1_poke
	incl	%ebx
	incl	%edi
	cmpl	$1600,%edi
	je		finish_draw
	jmp		loop_draw_screen
finish_draw:
	popl 	%edi
	popl	%esi
	popl	%ebx
	leave
		ret

# int mp1 ioctl startgame (unsigned long seed); 
/* The parameter seed must be used to seed the random number generator by passing the argument to 
* the seed generator. The two game boards should be dynamically allocated using mp1 malloc.  
If either allocation fails, free any allocated memory (from the ﬁrst allocation) 
and leave the two board pointers set to 0. 
After allocating the boards, ﬁll them both with 0 bytes, then call init virus on the current board. 
The return value from init virus is the initial value for infection. 
The aggression value should initially be set to 80, and population, which is measured in thousands of humans, 
should initially be 8,000,000. The function should return -1 if either allocation fails, or 0 if both succeed
*/
mp1_ioctl_startgame:
	pushl	%ebp	# save old frame pointer
	movl 	%esp,%ebp	# point to new frame
	pushl	%ebx
	pushl	%esi	
	pushl	%edi

	movl	8(%ebp),%eax	#	%eax is arg (seed)
	pushl	%eax
	call	seed_generator	# no return val, change rand_seed
	addl	$4,%esp
	pushl	$1600
#	void* mp1_malloc(int size)
	call	mp1_malloc
	addl	$4,%esp	# return the pointer
	cmpl	$0, %eax
	ja		next_malloc
	movl	$0,%eax
	movl	%eax,current_board	
	movl	%eax,next_board	# set pointer to null
	jmp		fail_malloc
next_malloc:
	movl	%eax,current_board	# put the pointer into current_board
	pushl	$1600
	call	mp1_malloc
	# void* mp1 malloc(unsigned long size);
	addl	$4,%esp
	cmpl	$0,%eax
	ja		finish_malloc
	movl	current_board,%eax
	pushl	%eax
	call	mp1_free
	addl	$4,%esp	# free current_board
	movl	$0,%eax
	movl	%eax,current_board
	movl	%eax,next_board
	jmp		fail_malloc
finish_malloc:
	movl	%eax,next_board
	movl	current_board,%eax
	movl	next_board,%ebx
	movl	$1600,%edi
loop_fillzero:
	movb	$0,(%eax)
	movb	$0,(%ebx)
	incl	%eax
	incl	%ebx
	decl	%edi
	cmpl	$0, %edi
	ja   	loop_fillzero
	movl	current_board,%ebx
	pushl	%ebx
	call 	init_virus
	addl	$4,%esp
 # The return value from init virus is the initial value for infection
	movl	%eax,infection
	movl	$80,%eax
	movl	%eax,aggression
	movl	$8000000,%eax
	movl	%eax,population
	movl	$0,%eax
	jmp		startgame_end
fail_malloc:
	movl	$-1,%eax
startgame_end:
	popl 	%edi
	popl	%esi
	popl	%ebx
	leave
		ret
 # int mp1 ioctl endgame (unsigned long ignore);
mp1_ioctl_endgame:
	pushl	%ebp	# save old frame pointer
	movl 	%esp,%ebp	# point to new frame
	pushl	%ebx
	pushl	%esi	
	pushl	%edi
	
	movl	current_board,%eax
	pushl	%eax
	call	mp1_free
	addl	$4,%esp
	movl	$0,%eax
	movl	%eax,current_board	# set to 0
	movl	next_board,%eax
	pushl	%eax
	call	mp1_free
	addl	$4,%esp
	movl	$0,%eax
	movl	%eax,next_board	# set to 0
	movl	$0,%eax

	popl 	%edi
	popl	%esi
	popl	%ebx
	leave
		ret
 	
mp1_ioctl_keystroke:
	pushl	%ebp	# save old frame pointer
	movl 	%esp,%ebp	# point to new frame
	pushl	%ebx
	pushl	%esi	
	pushl	%edi
# First, the function must copy the arguments from user space into kernel memory
	subl	$KA_STACK,%esp	# make room for the stack
	movl	%esp,%edi	# edi points to the allocated start
#	movl	$KA_SIZE, %eax	# eax is 14 , n = 14
	
	movl	8(%ebp),%eax	# %eax is arg
	cmpl	$0,%eax
	je		fail_to_copy
	pushl 	$KA_SIZE	# push 14
	pushl	%eax	# void* from, it's argument
	pushl	%edi
	call	mp1_copy_from_user
	addl	$12,%esp
	cmpl	$0,%eax
	jne		fail_to_copy
# The direction ﬁeld speciﬁes the direction of the keystroke: 0 for left, 1 for down, 2 for right, and 3 for up. 
# If left/right has been pressed, erase the current selector (a string is provided to you in mp1.S) 
/*struct keystroke_args {
    unsigned short direction;   2 bytes	 keystroke direction: 0 is left, 1 is down, 2 is right, 3 is up 
    unsigned short selector;	2 bytes  selector position (0 to 4)        
    unsigned char guess[5];	    5 bytes  current DNA fragment guess        
    unsigned char hint[5];	 	5 bytes  current DNA fragment hints        
}	*/
#	movl	%ebx,%esp
	movw	DIRECTION(%esp),%bx
	cmpw	$0,%bx
	je		selector_go_left
	cmpw	$1,%bx
	je		down_direction
	cmpw	$2,%bx
	je		selector_go_right
	cmpw	$3,%bx
	je		up_directinon
	jmp		fail_to_copy
# void mp1_poke(void);
# You can use this function to write to video memory.
#
# Interface: Register-based arguments (not C-style)
#    Inputs: %cl  - The byte you wish to write
#            %eax - Offset from the start of video memory that you wish
#                   to write to
#   Outputs: Text-mode video screen is written to at location %eax with
#            the byte in %cl
# Registers: Clobbers EDX
/*
mp1_poke:
		movl    vmem_base_addr(,1),%edx
		movb    %cl,(%edx,%eax,1)
		ret	*/
selector_go_left:	#	(14+10s,22)
# erase the current selector
	xorl	%eax,%eax
	xorl	%ebx,%ebx
	xorl	%ecx,%ecx
	xorl	%edx,%edx

	movb	select_blank,%cl	# %cl is the start of blank string
	movw	SELECTOR(%esp),%ax	# %ax is the selector
	movzx	%ax,%eax	
	imull	$10,%eax,%eax	# 10*s
	addl	$14,%eax    # 10*s+14
	addl	$1760,%eax	# 22*80+14+10s
	imull	$2,%eax,%eax
	call	mp1_poke
	movb	select_blank+1,%cl # +1
	addl	$2,%eax
	call	mp1_poke
	movb	select_blank+2,%cl
	addl	$2,%eax
	call	mp1_poke	# finish erasing
	movw	SELECTOR(%esp),%bx	
	cmpw	$0,%bx
	ja		no_cycle_to_right
	movw	$5,%bx
no_cycle_to_right:
	decw	%bx
	movw	%bx,2(%esp)	# write the new selector to stack
#	redraw the ==> at new position
	movb	select_image,%cl
	movw	SELECTOR(%esp),%ax
	movzx	%ax,%eax	
	imull	$10,%eax,%eax	# 10*s
	addl	$14,%eax    # 10*s+14
	addl	$1760,%eax	# 22*80+14+10s
	imull	$2,%eax,%eax
	call	mp1_poke
	addl	$2,%eax
	movb	1+select_image,%cl
	call	mp1_poke	# draw the second =
	movb	2+select_image,%cl
	addl	$2,%eax
	call	mp1_poke	# draw the third >	and finish redrawing
	jmp		real_keystroke_end
selector_go_right:
	movb	select_blank,%cl	# %al is the start of blank string
	movw	2(%esp),%ax	# %ax is the selector
	movzx	%ax,%eax	
	imull	$10,%eax,%eax	# 10*s
	addl	$14,%eax    # 10*s+14
	addl	$1760,%eax	# 22*80+14+10s
	imull	$2,%eax,%eax
	call	mp1_poke
	addl	$2,%eax
	movb	select_blank+1,%cl	# %al is the start of blank string
	call	mp1_poke
	addl	$2,%eax
	movb	select_blank+2,%cl
	call	mp1_poke	# finish erasing
	xorl	%ebx,%ebx
	movw	2(%esp),%bx
	cmpw	$4,%bx
	jb		no_cycle_to_left
	movw	$-1,%bx
no_cycle_to_left:
	addw	$1,%bx
	movw	%bx,2(%esp)	# write the selector to the stack
	movb	select_image,%cl
	movw	2(%esp),%ax
	movzx	%ax,%eax	
	imull	$10,%eax,%eax	# 10*s
	addl	$14,%eax    # 10*s+14
	addl	$1760,%eax	# 22*80+14+10s
	imull	$2,%eax,%eax
	movb	select_image,%cl
	call	mp1_poke
	addl	$2,%eax
	movb	select_image+1,%cl
	call	mp1_poke	# draw the second =
	movb	2+select_image,%cl
	addl	$2,%eax
	call	mp1_poke	# draw the third >	and finish redrawing
/*struct keystroke_args {
    unsigned short direction;   2 bytes	 keystroke direction: 0 is left, 1 is down, 2 is right, 3 is up 
    unsigned short selector;	2 bytes  selector position (0 to 4)        
    unsigned char guess[5];	    5 bytes  current DNA fragment guess        
    unsigned char hint[5];	 	5 bytes  current DNA fragment hints        
}	*/
	jmp		real_keystroke_end
up_directinon:
	movw	2(%esp),%ax	# %ax is the selector
	movzx	%ax,%eax
	 # bl is current value of guess
	movb	GUESS(%esp,%eax,1),%bl
find_base_up:
	addb    $1,%bl	
	cmpb	$4,%bl	# decide wther to wrap to zero
	jb		not_down_to_zero
	movb	$0,%bl
not_down_to_zero:
	movb	HINT(%esp,%eax,1),%cl	# cl is the curent hint
	movb	$1,%dl
	pushl	%ecx
	movb	%bl,%cl		# bug: need to use cl 
	sal  	%cl,%dl
	popl	%ecx
	andb	%cl,%dl
	cmpb	$0,%dl
	je		find_base_up		# whether we should write %bl to guess when not included
	movb	%bl,GUESS(%esp,%eax,1)  # update guess
	movzx	%bl,%ebx
	movb	base_array(%ebx),%cl
	movw	2(%esp),%ax	# %ax is the selector
	movzx	%ax,%eax	
	imull	$10,%eax,%eax	# 10*s
	addl	$18,%eax    # 10*s+18
	addl	$1760,%eax	# 22*80+18+10s
	imull	$2,%eax,%eax
	call	mp1_poke	
	jmp		real_keystroke_end
down_direction:
	movw	2(%esp),%ax	# %ax is the selector
	movzx	%ax,%eax
	 # bl is current value of guess
	movb	GUESS(%esp,%eax,1),%bl
find_base_down:
	cmpb	$0,%bl
	jne		not_up_to_three
	movb	$4,%bl
not_up_to_three:
	subb	$1,%bl
	movb	HINT(%esp,%eax,1),%cl	# cl is the curent hint
	movb	$1,%dl
	pushl	%ecx
	movb	%bl,%cl
	sal  	%cl,%dl	# bit comparision 
	popl	%ecx
	andb	%cl,%dl
	cmpb	$0,%dl
	je		find_base_down		# whether we should write %bl to guess when not included
	movb	%bl,GUESS(%esp,%eax,1)  # update guess
	movzx	%bl,%ebx
	movb	base_array(%ebx),%cl
	movw	2(%esp),%ax	# %ax is the selector
	movzx	%ax,%eax	
	imull	$10,%eax,%eax	# 10*s
	addl	$18,%eax    # 10*s+18
	addl	$1760,%eax	# 22*80+18+10s
	imull	$2,%eax,%eax
	call	mp1_poke	
	jmp		real_keystroke_end
real_keystroke_end:
	/* Finally, regardless of the direction of the keystroke, the function must
	copy the modified keystroke args structure from the kernel back into user space (at the same location given by the
	argument)
	*/
	# use mp1 copy to user (void *to, const void *from, unsigned long n);
	pushl	$KA_SIZE
	pushl	%edi
	pushl	8(%ebp)
	call	mp1_copy_to_user
	addl	$12,%esp
	cmpl	$0,%eax
	jne		fail_to_copy
	movl	$0,%eax
keystroke_end:
	addl	$16,%esp
	popl 	%edi
	popl	%esi
	popl	%ebx
	leave
		ret

fail_to_copy:
	movl	$-1,%eax

	jmp		keystroke_end

/*struct game_status {
    unsigned int pop;		
    unsigned int inf;		
}*/
# int mp1 ioctl getstatus (unsigned long* user status);
mp1_ioctl_getstatus:
	pushl	%ebp	# save old frame pointer
	movl 	%esp,%ebp	# point to new frame
	pushl	%ebx
	pushl	%esi	
	pushl	%edi
	subl	$GS_SIZE,%esp	# make room for gamesatus
	movl	population,%eax	
	movl	infection,%ebx
	movl	%eax,(%esp)
	movl	%ebx,4(%esp)

	movl	8(%ebp),%ebx
	cmpl	$0,%ebx
	je		fail_copy_to_user
	
	movl	%esp,%ebx
	pushl	$GS_SIZE
	pushl	%ebx
	pushl	8(%ebp)	# bug here
	call	mp1_copy_to_user
	addl	$12,%esp
	cmpl	$0,%eax
	ja   	fail_copy_to_user
getstatus_end:
	addl	$8,%esp
	popl 	%edi
	popl	%esi
	popl	%ebx
	leave	
		ret
fail_copy_to_user:
	movl	$-1,%eax

	jmp		getstatus_end



	# int mp1 ioctl vaccinate (unsigned long packed args)
#  packed args is a 32-bit integer containing two unsigned 16-bit integers
#  The low 16 bits contain the percentage chance that a live cell in the current board should be killed (set to 0)
/* For each cell in the board, call the generate function to generate a 32-bit unsigned random number, 
calculate the remainder of that number when divided by 100, then compare with the given percentage to 
determine whether or not to kill thnned amount that must be added to the aggression variable, 
increasing the rate at which the virus expands. You may ignore the possibility of overﬂow. 
This function should always return 0. */
mp1_ioctl_vaccinate:
	pushl	%ebp	# save old frame pointer
	movl 	%esp,%ebp	# point to new frame
	pushl	%ebx
	pushl	%esi	
	pushl	%edi

	movl	current_board,%ebx
	movl	$1600,%edi
	
loop_vaccinate:
	call	generate
# 	%eax is dividend
	movl	$0,%edx
	movl	$100,%esi
	div 	%esi
# remainder is in %edx
	xorl	%ecx,%ecx
	movl	8(%ebp),%ecx
	andl	$0x0000FFFF,%ecx
	cmpl	%ecx,%edx
	jae		not_kill_cell
	movb	$0,(%ebx)
not_kill_cell:
	incl	%ebx
	decl	%edi
	cmpl	$0,%edi
	ja  	loop_vaccinate

	xorl	%ecx,%ecx
	movl	8(%ebp),%ecx
	shrl	$16,%ecx
	addl	%ecx,aggression

	movl	$0,%eax
	popl 	%edi
	popl	%esi
	popl	%ebx
	leave
		ret

