void __assert_func(const char *_file, int _line, const char *_func,
const char *_expr )
{
(void)_file;
(void)_line;
(void)_func;
(void)_expr;
volatile int loop = 1;
__asm volatile ("bkpt 0");
do { ; } while( loop );
}
