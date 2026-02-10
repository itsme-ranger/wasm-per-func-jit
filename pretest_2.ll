; ModuleID = 'pretest_2'
source_filename = "pretest_2.ll"
target triple = "x86_64-apple-macos13"

; GLOBAL VAR

@call_count = global i32 0
@max_depth = constant i32 10

; EXTERNAL DECLARATIONS

declare i32 @putchar(i32)

; HELPER FUNCTIONS (leaf nodes)

; Increment global counter - simulates tracking JIT compilations
define void @track_call() {
entry:
  %count = load i32, i32* @call_count
  %new_count = add i32 %count, 1
  store i32 %new_count, i32* @call_count
  ret void
}

; Print single char
define void @emit_char(i32 %c) {
entry:
  call void @track_call()
  call i32 @putchar(i32 %c)
  ret void
}

; FUNCTIONS WITH CONTROL FLOW

; Conditional: absolute value
define i32 @abs(i32 %x) {
entry:
  call void @track_call()
  %is_neg = icmp slt i32 %x, 0
  br i1 %is_neg, label %negate, label %done

negate:
  %neg_x = sub i32 0, %x
  br label %done

done:
  %result = phi i32 [%x, %entry], [%neg_x, %negate]
  ret i32 %result
}

; Loop: sum from 1 to n
define i32 @sum_to_n(i32 %n) {
entry:
  call void @track_call()
  br label %loop

loop:
  %i = phi i32 [1, %entry], [%next_i, %loop]
  %acc = phi i32 [0, %entry], [%next_acc, %loop]
  %next_acc = add i32 %acc, %i
  %next_i = add i32 %i, 1
  %done = icmp sgt i32 %next_i, %n
  br i1 %done, label %exit, label %loop

exit:
  ret i32 %next_acc
}

; RECURSIVE FUNC

; Recursive factorial (call graph traversal)
define i32 @factorial(i32 %n) {
entry:
  call void @track_call()
  %is_base = icmp sle i32 %n, 1
  br i1 %is_base, label %base_case, label %recurse

base_case:
  ret i32 1

recurse:
  %n_minus_1 = sub i32 %n, 1
  %sub_result = call i32 @factorial(i32 %n_minus_1)
  %result = mul i32 %n, %sub_result
  ret i32 %result
}

; INDIRECT CALL TARGET

define i32 @op_add(i32 %a, i32 %b) {
entry:
  call void @track_call()
  %r = add i32 %a, %b
  ret i32 %r
}

define i32 @op_mul(i32 %a, i32 %b) {
entry:
  call void @track_call()
  %r = mul i32 %a, %b
  ret i32 %r
}

; MORE COMPLEX FUNCTIONS (e.g. calls multiple others)

; calls multiple functions
define i32 @compute(i32 %x) {
entry:
  %abs_x = call i32 @abs(i32 %x)
  %sum = call i32 @sum_to_n(i32 %abs_x)
  %fact = call i32 @factorial(i32 5)
  %combined = call i32 @op_add(i32 %sum, i32 %fact)
  ret i32 %combined
}

; UNUSED FUNCTION (shouldn't be compiled in lazy JIT)

define i32 @never_called(i32 %x) {
entry:
  %result = mul i32 %x, %x
  %again = mul i32 %result, %x
  ret i32 %again
}

; ENTRY POINT

define i32 @main() {
entry:
  %result = call i32 @compute(i32 -7)
  call void @emit_char(i32 10)
  ret i32 %result
}