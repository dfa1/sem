typedef void *yyscan_t;

extern int yylex_init(yyscan_t * scanner);
extern int yylex_destroy(yyscan_t yyscanner);
extern FILE *yyget_in(yyscan_t yyscanner);
extern void yyset_in(FILE * in_str, yyscan_t yyscanner);
extern FILE *yyget_out(yyscan_t yyscanner);
extern void yyset_out(FILE * out_str, yyscan_t yyscanner);
extern char *yyget_text(yyscan_t yyscanner);
extern int yyget_lineno(yyscan_t yyscanner);
extern void yyset_lineno(int line_number, yyscan_t yyscanner);
extern int yyget_column(yyscan_t yyscanner);
extern void yyset_column(int column_no, yyscan_t yyscanner);
extern int yylex(yyscan_t yyscanner);
