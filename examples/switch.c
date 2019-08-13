int main(int argc, char** argv) {
    int i = 5;
    switch (i)
    {
        case 0:
            i = 10;
            i += 1;
            break;
        case 1:
            i = 20;
            break;
        default:
            i = 0;
            break;
    }
    return 0;
}

