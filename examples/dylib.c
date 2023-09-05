void* test_func(long parameter1, long parameter2) {
    long *mem = (long *)malloc(sizeof(parameter1) + sizeof(parameter2));
    mem[0] = parameter1;
    mem[1] = parameter2;
    return (void *)mem;
}

int main(int argc, char **argv) {
    free(test_func(42, 43));
    return 0;
}
