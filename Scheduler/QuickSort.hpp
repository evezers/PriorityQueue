//
// Created by evezers on 12/05/23.
//

#ifndef PRIORITYQUEUE_QUICKSORT_HPP
#define PRIORITYQUEUE_QUICKSORT_HPP


template <typename T>
void swap(T* a, T* b)
{
    T t = *a;
    *a = *b;
    *b = t;
}

template <typename T>
int partition(T arr[], int l, int h)
{
    T x = arr[h];
    int i = (l - 1);

    for (int j = l; j <= h - 1; j++) {
        if (arr[j] <= x) {
            i++;
            swap(&arr[i], &arr[j]);
        }
    }
    swap(&arr[i + 1], &arr[h]);
    return (i + 1);
}

template <typename T>
void quickSortIterative(T arr[], int l, int h)
{
    int stack[h - l + 1];

    int top = -1;

    stack[++top] = l;
    stack[++top] = h;

    while (top >= 0) {
        h = stack[top--];
        l = stack[top--];

        int p = partition(arr, l, h);

        if (p - 1 > l) {
            stack[++top] = l;
            stack[++top] = p - 1;
        }

        if (p + 1 < h) {
            stack[++top] = p + 1;
            stack[++top] = h;
        }
    }
}

#endif //PRIORITYQUEUE_QUICKSORT_HPP
