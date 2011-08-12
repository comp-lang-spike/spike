
	.text

/*** "x++", "++x" ***/

SpikeOperSucc:
	.globl	SpikeOperSucc
	movl	$__sym___succ__, %edx	# selector
	movl	$0, %ecx		# argument count
	jmp	SpikeSendMessage

SpikeOperSuccSuper:
	.globl	SpikeOperSuccSuper
	movl	$__sym___succ__, %edx	# selector
	movl	$0, %ecx		# argument count
	jmp	SpikeSendMessageSuper


/*** "x--", "--x" ***/

SpikeOperPred:
	.globl	SpikeOperPred
	movl	$__sym___pred__, %edx	# selector
	movl	$0, %ecx		# argument count
	jmp	SpikeSendMessage

SpikeOperPredSuper:
	.globl	SpikeOperPredSuper
	movl	$__sym___pred__, %edx	# selector
	movl	$0, %ecx		# argument count
	jmp	SpikeSendMessageSuper


/*** "&x" ***/

SpikeOperAddr:
	.globl	SpikeOperAddr
	movl	$__sym___addr__, %edx	# selector
	movl	$0, %ecx		# argument count
	jmp	SpikeSendMessage

SpikeOperAddrSuper:
	.globl	SpikeOperAddrSuper
	movl	$__sym___addr__, %edx	# selector
	movl	$0, %ecx		# argument count
	jmp	SpikeSendMessageSuper


/*** "+x" ***/

SpikeOperPos:
	.globl	SpikeOperPos
	movl	$__sym___pos__, %edx	# selector
	movl	$0, %ecx		# argument count
	jmp	SpikeSendMessage

SpikeOperPosSuper:
	.globl	SpikeOperPosSuper
	movl	$__sym___pos__, %edx	# selector
	movl	$0, %ecx		# argument count
	jmp	SpikeSendMessageSuper


/*** "-x" ***/

SpikeOperNeg:
	.globl	SpikeOperNeg
	movl	$__sym___neg__, %edx	# selector
	movl	$0, %ecx		# argument count
	jmp	SpikeSendMessage

SpikeOperNegSuper:
	.globl	SpikeOperNegSuper
	movl	$__sym___neg__, %edx	# selector
	movl	$0, %ecx		# argument count
	jmp	SpikeSendMessageSuper


/*** "~x" ***/

SpikeOperBNeg:
	.globl	SpikeOperBNeg
	movl	$__sym___bneg__, %edx	# selector
	movl	$0, %ecx		# argument count
	jmp	SpikeSendMessage

SpikeOperBNegSuper:
	.globl	SpikeOperBNegSuper
	movl	$__sym___bneg__, %edx	# selector
	movl	$0, %ecx		# argument count
	jmp	SpikeSendMessageSuper


/*** "!x" ***/

SpikeOperLNeg:
	.globl	SpikeOperLNeg
	movl	$__sym___lneg__, %edx	# selector
	movl	$0, %ecx		# argument count
	jmp	SpikeSendMessage

SpikeOperLNegSuper:
	.globl	SpikeOperLNegSuper
	movl	$__sym___lneg__, %edx	# selector
	movl	$0, %ecx		# argument count
	jmp	SpikeSendMessageSuper


/*** "x * y" ***/

SpikeOperMul:
	.globl	SpikeOperMul
	movl	$__sym___mul__, %edx	# selector
	movl	$1, %ecx		# argument count
	jmp	SpikeSendMessage

SpikeOperMulSuper:
	.globl	SpikeOperMulSuper
	movl	$__sym___mul__, %edx	# selector
	movl	$1, %ecx		# argument count
	jmp	SpikeSendMessageSuper


/*** "x / y" ***/

SpikeOperDiv:
	.globl	SpikeOperDiv
	movl	$__sym___div__, %edx	# selector
	movl	$1, %ecx		# argument count
	jmp	SpikeSendMessage

SpikeOperDivSuper:
	.globl	SpikeOperDivSuper
	movl	$__sym___div__, %edx	# selector
	movl	$1, %ecx		# argument count
	jmp	SpikeSendMessageSuper


/*** "x % y" ***/

SpikeOperMod:
	.globl	SpikeOperMod
	movl	$__sym___mod__, %edx	# selector
	movl	$1, %ecx		# argument count
	jmp	SpikeSendMessage

SpikeOperModSuper:
	.globl	SpikeOperModSuper
	movl	$__sym___mod__, %edx	# selector
	movl	$1, %ecx		# argument count
	jmp	SpikeSendMessageSuper


/*** "x + y" ***/

SpikeOperAdd:
	.globl	SpikeOperAdd
	movl	$__sym___add__, %edx	# selector
	movl	$1, %ecx		# argument count
	jmp	SpikeSendMessage

SpikeOperAddSuper:
	.globl	SpikeOperAddSuper
	movl	$__sym___add__, %edx	# selector
	movl	$1, %ecx		# argument count
	jmp	SpikeSendMessageSuper


/*** "x - y" ***/

SpikeOperSub:
	.globl	SpikeOperSub
	movl	$__sym___sub__, %edx	# selector
	movl	$1, %ecx		# argument count
	jmp	SpikeSendMessage

SpikeOperSubSuper:
	.globl	SpikeOperSubSuper
	movl	$__sym___sub__, %edx	# selector
	movl	$1, %ecx		# argument count
	jmp	SpikeSendMessageSuper


/*** "x << y" ***/

SpikeOperLShift:
	.globl	SpikeOperLShift
	movl	$__sym___lshift__, %edx	# selector
	movl	$1, %ecx		# argument count
	jmp	SpikeSendMessage

SpikeOperLShiftSuper:
	.globl	SpikeOperLShiftSuper
	movl	$__sym___lshift__, %edx	# selector
	movl	$1, %ecx		# argument count
	jmp	SpikeSendMessageSuper


/*** "x >> y" ***/

SpikeOperRShift:
	.globl	SpikeOperRShift
	movl	$__sym___rshift__, %edx	# selector
	movl	$1, %ecx		# argument count
	jmp	SpikeSendMessage

SpikeOperRShiftSuper:
	.globl	SpikeOperRShiftSuper
	movl	$__sym___rshift__, %edx	# selector
	movl	$1, %ecx		# argument count
	jmp	SpikeSendMessageSuper


/*** "x < y" ***/

SpikeOperLT:
	.globl	SpikeOperLT
	movl	$__sym___lt__, %edx	# selector
	movl	$1, %ecx		# argument count
	jmp	SpikeSendMessage

SpikeOperLTSuper:
	.globl	SpikeOperLTSuper
	movl	$__sym___lt__, %edx	# selector
	movl	$1, %ecx		# argument count
	jmp	SpikeSendMessageSuper


/*** "x > y" ***/

SpikeOperGT:
	.globl	SpikeOperGT
	movl	$__sym___gt__, %edx	# selector
	movl	$1, %ecx		# argument count
	jmp	SpikeSendMessage

SpikeOperGTSuper:
	.globl	SpikeOperGTSuper
	movl	$__sym___gt__, %edx	# selector
	movl	$1, %ecx		# argument count
	jmp	SpikeSendMessageSuper


/*** "x <= y" ***/

SpikeOperLE:
	.globl	SpikeOperLE
	movl	$__sym___le__, %edx	# selector
	movl	$1, %ecx		# argument count
	jmp	SpikeSendMessage

SpikeOperLESuper:
	.globl	SpikeOperLESuper
	movl	$__sym___le__, %edx	# selector
	movl	$1, %ecx		# argument count
	jmp	SpikeSendMessageSuper


/*** "x >= y" ***/

SpikeOperGE:
	.globl	SpikeOperGE
	movl	$__sym___ge__, %edx	# selector
	movl	$1, %ecx		# argument count
	jmp	SpikeSendMessage

SpikeOperGESuper:
	.globl	SpikeOperGESuper
	movl	$__sym___ge__, %edx	# selector
	movl	$1, %ecx		# argument count
	jmp	SpikeSendMessageSuper


/*** "x == y" ***/

SpikeOperEq:
	.globl	SpikeOperEq
	movl	$__sym___eq__, %edx	# selector
	movl	$1, %ecx		# argument count
	jmp	SpikeSendMessage

SpikeOperEqSuper:
	.globl	SpikeOperEqSuper
	movl	$__sym___eq__, %edx	# selector
	movl	$1, %ecx		# argument count
	jmp	SpikeSendMessageSuper


/*** "x != y" ***/

SpikeOperNE:
	.globl	SpikeOperNE
	movl	$__sym___ne__, %edx	# selector
	movl	$1, %ecx		# argument count
	jmp	SpikeSendMessage

SpikeOperNESuper:
	.globl	SpikeOperNESuper
	movl	$__sym___ne__, %edx	# selector
	movl	$1, %ecx		# argument count
	jmp	SpikeSendMessageSuper


/*** "x & y" ***/

SpikeOperBAnd:
	.globl	SpikeOperBAnd
	movl	$__sym___band__, %edx	# selector
	movl	$1, %ecx		# argument count
	jmp	SpikeSendMessage

SpikeOperBAndSuper:
	.globl	SpikeOperBAndSuper
	movl	$__sym___band__, %edx	# selector
	movl	$1, %ecx		# argument count
	jmp	SpikeSendMessageSuper


/*** "x ^ y" ***/

SpikeOperBXOr:
	.globl	SpikeOperBXOr
	movl	$__sym___bxor__, %edx	# selector
	movl	$1, %ecx		# argument count
	jmp	SpikeSendMessage

SpikeOperBXOrSuper:
	.globl	SpikeOperBXOrSuper
	movl	$__sym___bxor__, %edx	# selector
	movl	$1, %ecx		# argument count
	jmp	SpikeSendMessageSuper


/*** "x | y" ***/

SpikeOperBOr:
	.globl	SpikeOperBOr
	movl	$__sym___bor__, %edx	# selector
	movl	$1, %ecx		# argument count
	jmp	SpikeSendMessage

SpikeOperBOrSuper:
	.globl	SpikeOperBOrSuper
	movl	$__sym___bor__, %edx	# selector
	movl	$1, %ecx		# argument count
	jmp	SpikeSendMessageSuper


/*** "f(x)" ***/

SpikeCall:
	.globl	SpikeCall
	movl	$__sym___apply__, %edx	# selector
	jmp	SpikeSendMessage

SpikeCallSuper:
	.globl	SpikeCallSuper
	movl	$__sym___apply__, %edx	# selector
	jmp	SpikeSendMessageSuper


/*** "a[i]" ***/

SpikeGetIndex:
	.globl	SpikeGetIndex
	movl	$__sym___index__, %edx	# selector
	jmp	SpikeSendMessage

SpikeGetIndexSuper:
	.globl	SpikeGetIndexSuper
	movl	$__sym___index__, %edx	# selector
	jmp	SpikeSendMessageSuper


/*** "a[i] = v" ***/

SpikeSetIndex:
	.globl	SpikeSetIndex
	movl	$__sym___index__, %edx	# selector
	jmp	SpikeSendMessageLValue

SpikeSetIndexSuper:
	.globl	SpikeSetIndexSuper
	movl	$__sym___index__, %edx	# selector
	jmp	SpikeSendMessageSuperLValue


/*** "*p" ***/

SpikeGetInd:
	.globl	SpikeGetInd
	movl	$__sym___ind__, %edx	# selector
	movl	$0, %ecx		# argument count
	jmp	SpikeSendMessage

SpikeGetIndSuper:
	.globl	SpikeGetIndSuper
	movl	$__sym___ind__, %edx	# selector
	movl	$0, %ecx		# argument count
	jmp	SpikeSendMessageSuper


/*** "*p = v" ***/

SpikeSetInd:
	.globl	SpikeSetInd
	movl	$__sym___ind__, %edx	# selector
	movl	$1, %ecx		# argument count
	jmp	SpikeSendMessageLValue

SpikeSetIndSuper:
	.globl	SpikeSetIndSuper
	movl	$__sym___ind__, %edx	# selector
	movl	$1, %ecx		# argument count
	jmp	SpikeSendMessageSuperLValue


/*** "obj.attr", "obj.*attr" ***/

SpikeGetAttr:
	.globl	SpikeGetAttr
	movl	$0, %ecx		# argument count
	jmp	SpikeSendMessage

SpikeGetAttrSuper:
	.globl	SpikeGetAttrSuper
	movl	$0, %ecx		# argument count
	jmp	SpikeSendMessageSuper


/*** "obj.attr = v", "obj.*attr = v" ***/

SpikeSetAttr:
	.globl	SpikeSetAttr
	movl	$1, %ecx		# argument count
	jmp	SpikeSendMessageLValue

SpikeSetAttrSuper:
	.globl	SpikeSetAttrSuper
	movl	$1, %ecx		# argument count
	jmp	SpikeSendMessageSuperLValue


/* helper routines */

getClass:
	cmpl	$0, %esi	# test for null
	jne	.L11

	movl	$Null, %ebx
	ret
.L11:
	movl	%esi, %eax
	andl	$3, %eax	# test for object pointer
	cmpl	$0, %eax
	jne	.L12

	movl	(%esi), %ebx 	# get class
	ret
.L12:
	cmpl	$2, %eax	# test for SmallInteger
	jne	.L13

	movl	$Integer, %ebx
	ret

.L13:
	pushl	$__sym_badObjectPointer
	call	SpikeError
	movl	$0, %ebx
	ret


/*
 * send message bottleneck
 *
 * On entry:
 *     %edx: selector
 *     %ecx: argument count
 */

SpikeSendMessageSuperLValue:
	.globl	SpikeSendMessageSuper
	pushl	%ebp		# create new stack frame
	movl	%esp, %ebp
	pushl	%ebx		# save registers
	pushl	%esi
	pushl	%edi
	movl	4(%ebx), %ebx 	# get superclass
	movl	$1, %edi	# lvalue namespace
	jmp	lookupMethod

SpikeSendMessageLValue:
	.globl	SpikeSendMessage
	pushl	%ebp		# create new stack frame
	movl	%esp, %ebp
	pushl	%ebx		# save registers
	pushl	%esi
	pushl	%edi
	movl	8(%ebp,%ecx,4), %esi  # get receiver
	call	getClass 	# get class
	movl	$1, %edi	# lvalue namespace
	jmp	lookupMethod

SpikeSendMessageSuper:
	.globl	SpikeSendMessageSuper
	pushl	%ebp		# create new stack frame
	movl	%esp, %ebp
	pushl	%ebx		# save registers
	pushl	%esi
	pushl	%edi
	movl	4(%ebx), %ebx 	# get superclass
	movl	$0, %edi	# rvalue namespace
	jmp	lookupMethod

SpikeSendMessage:
	.globl	SpikeSendMessage
	pushl	%ebp		# create new stack frame
	movl	%esp, %ebp
	pushl	%ebx		# save registers
	pushl	%esi
	pushl	%edi
	movl	8(%ebp,%ecx,4), %esi  # get receiver
	call	getClass 	# get class
	movl	$0, %edi	# rvalue namespace
	/* fall through */

lookupMethod:
	pushl	%ecx		# save registers
	pushl	%edx
.L1:
	pushl	(%esp)		# selector
	pushl	%edi		# namespace
	pushl	%ebx		# behavior
	call	SpikeLookupMethod
	addl	$12, %esp
	testl	%eax, %eax
	jne	callNewMethod
	movl	4(%ebx), %ebx 	# up superclass chain
	testl	%ebx, %ebx
	jne	.L1

/* XXX createActualMessage */
	pushl	$__sym_doesNotUnderstand
	call	SpikeError
	jmp	lookupMethod

/* found it */
callNewMethod:
	popl	%edx		# restore selector
	popl	%ecx		# restore argumentCount

SpikeCallNewMethod: /* entry point for Function __apply__ */
	.globl	SpikeCallNewMethod
	cmpl	4(%eax), %ecx	# minArgumentCount
	jb	wrongNumberOfArguments
	cmpl	8(%eax), %ecx	# maxArgumentCount
	ja	wrongNumberOfArguments

/* allocate and initialize locals */
	movl	%ecx, %edi	# save argumentCount
	movl	12(%eax), %ecx	# localCount
	cmpl	$0, %ecx
	je	.L3
.L2:
	pushl	$0
	loop	.L2
.L3:
	movl	%edi, %ecx	# restore argumentCount

/* compute code entry point */
	addl	$16, %eax	# skip Method object header
	pushl	%eax		# save entry point for later

/* set up instance variable pointer in %edi */
	movl	%ebx, %edi 	# get class
	movl	$0, %eax 	# tally instance vars...
	jmp	.L5 		# ...in superclasses
.L4:
	addl	16(%edi), %eax	# instVarCount
.L5:
	movl	4(%edi), %edi 	# up superclass chain
	testl	%edi, %edi
	jne	.L4

	leal	4(%esi,%eax,4), %edi

/*
 * call the method
 *
 * On entry:
 *     %eax: undefined (instVarTally of superclasses)
 *     %ecx: argumentCount
 *     %edx: selector
 *     %ebx: methodClass
 *     %esi: self
 *     %edi: instVarPointer
 *
 *     %esp/%ebp point to a fully initialized stack frame
 */
	ret			# call it

wrongNumberOfArguments:
	pushl	$__sym_wrongNumberOfArguments
	call	SpikeError
