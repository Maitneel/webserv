while True:
    a = list(input().split())
    if (len(a) == 9):
        break;

    for i in range(len(a)):
        a[i] = "0x" + a[i];

    print(", ".join(a));