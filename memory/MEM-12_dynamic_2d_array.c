/*
 * MEM-12: Dynamically allocate a 2D array (rows x cols)
 * Method 1: array of row pointers, each row malloc'd separately (rows+1 mallocs).
 * Method 2: ONE malloc - row pointer table followed by contiguous data (1 free).
 * Method 3: pointer to VLA / fixed-width array: int (*m)[cols] (C99, simplest).
 * Interview points: freeing correctly (rows first, then table), cache friendliness
 * of contiguous memory, checking every malloc for NULL and cleaning up on failure.
 */
#include <stdio.h>
#include <stdlib.h>

/* Method 1 */
int **alloc2d_rows(int rows, int cols)
{
    int r;
    int **m = malloc(rows * sizeof *m);
    if (!m)
        return NULL;
    for (r = 0; r < rows; r++) {
        m[r] = malloc(cols * sizeof **m);
        if (!m[r]) {                    /* cleanup on partial failure */
            while (r--)
                free(m[r]);
            free(m);
            return NULL;
        }
    }
    return m;
}

void free2d_rows(int **m, int rows)
{
    int r;
    if (!m)
        return;
    for (r = 0; r < rows; r++)
        free(m[r]);
    free(m);
}

/* Method 2: single allocation, still usable as m[r][c] */
int **alloc2d_single(int rows, int cols)
{
    int r;
    int **m = malloc(rows * sizeof(int *) + (size_t)rows * cols * sizeof(int));
    if (!m)
        return NULL;
    int *data = (int *)(m + rows);      /* data starts right after the pointer table */
    for (r = 0; r < rows; r++)
        m[r] = data + r * cols;
    return m;                           /* caller does a single free(m) */
}

static void fill_print(int **m, int rows, int cols, const char *tag)
{
    int r, c;
    printf("%s:\n", tag);
    for (r = 0; r < rows; r++) {
        for (c = 0; c < cols; c++) {
            m[r][c] = r * cols + c;
            printf("%3d", m[r][c]);
        }
        printf("\n");
    }
}

int main(void)
{
    int rows = 3, cols = 4, r, c;

    int **a = alloc2d_rows(rows, cols);
    fill_print(a, rows, cols, "Method 1 (row pointers)");
    free2d_rows(a, rows);

    int **b = alloc2d_single(rows, cols);
    fill_print(b, rows, cols, "Method 2 (single malloc)");
    free(b);

    int (*v)[cols] = malloc(sizeof(int[rows][cols]));   /* Method 3 (C99 VLA type) */
    printf("Method 3 (pointer to VLA):\n");
    for (r = 0; r < rows; r++) {
        for (c = 0; c < cols; c++) {
            v[r][c] = r * 10 + c;
            printf("%3d", v[r][c]);
        }
        printf("\n");
    }
    free(v);
    return 0;
}
