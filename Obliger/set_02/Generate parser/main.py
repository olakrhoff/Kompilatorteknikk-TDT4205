from curses.ascii import islower


def generate():
    file1 = open('data.txt', 'r')
    Lines = file1.readlines()

    count = 0
    # Strips the newline character
    for line in Lines:
        count += 1
        parts = line.split(" ")
        print(parts[0] + " :")
        args = parts[1:]
        more = False
        for p in args:
            if p.islower():
                print(p +"{\n$$ = malloc(sizeof(node_t));\nnode_init($$, FUNCTION, NULL, );\n}")
                if more:
                    print("|\n")
                more = True

        print(";")


if __name__ == '__main__':
    generate()
