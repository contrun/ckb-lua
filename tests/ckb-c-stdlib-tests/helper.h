
#ifndef __HELPER_H__
#define __HELPER_H__

#define ASSERT(cond)                                                           \
    do {                                                                       \
        if (!(cond)) {                                                         \
            printf("test failed at file: %s, line: %d\n", __FILE__, __LINE__); \
            ckb_exit(-1);                                                      \
        }                                                                      \
    } while (0)

#define ASSERT_EQ(a, b) ASSERT((a) == (b))

#endif // __HELPER_H__
