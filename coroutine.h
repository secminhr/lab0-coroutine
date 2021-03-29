#define cor_func static int __first_call = 1;\
if (!__first_call) {\
    goto cor;\
}\

#define cor_start __first_call = 0;\
static int __con = 0;\
cor:\
switch(__con) {\
case 0: {

#define yield __con = 10;\
return 

#define cor_end }\
case 10:\
__con=0;\
return;\
}

#define cor_end_with_value(val) }\
case 10: \
__con = 0;\
return val; \
}

