#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include <math.h>
#include <string.h>

#define FALSE 0
#define TRUE 1

//kandydat na liczbe pierwsza, deleted oznacza wykreslenie w sicie
typedef struct node
{
    int num;
    int deleted;
}node;

//struktura przekazywana do threada, otrzymuje zakres tablicy liczb i dzielnik
typedef struct thread_data
{
    node *numbers;
    int start;
    int stop;
    int div;
    int numbers_deleted;
}thread_data;

void* calculatePrime(void* input)
{
    thread_data *data = (thread_data*)input;
    
    //sprawdz czy liczba jest wielokrotnoscia dzielnika okreslonego w mainie i wykresl jesli tak
    for(int i = data->start; i <= data->stop; i++)
    {
        if(data->numbers[i].deleted == TRUE) continue;
        if(data->numbers[i].num / data->div != 1 && data->numbers[i].num % data->div == 0) 
        {
            data->numbers[i].deleted = TRUE;
            data->numbers_deleted++;
        }
    }

    return (void*) data;
}

float calculateTimeDiff(struct timeval *start, struct timeval *end)
{
    return (end->tv_sec - start->tv_sec) + 1e-6*(end->tv_usec - start->tv_usec);
}

//funkcja rysujaca licznik postepu
void drawProgress(int percent)
{
    fflush(stdout);
    printf("\r[");

    for (int i = 0; i < 25; i++) 
    {
        if(i * 4 < percent) printf("#");
        else printf("=");
    }

    printf("] %d%%", percent);
}

int main(int argc, char **argv)
{
    int lenght, threads_available, lenght_div, lenght_odd, deleted_sum = 0;
    char file_name[256];
    node *numbers;
    pthread_t *threads;

    if(argc == 4)
    {
        lenght = atoi(argv[1]);
        threads_available = atoi(argv[2]);
        strcpy(file_name, argv[3]);
    }

    else
    {
        printf("Podaj: [liczbe liczb do wyznaczenia] [liczbe watkow] [nazwe pliku zapisu] >> ");
        scanf("%d %d %s", &lenght, &threads_available, file_name);
    }

    //podzielenie liczb na ilosc watkow, oraz ustalenie niepodzielnych liczb
    lenght_div = lenght / threads_available;
    lenght_odd = lenght % threads_available;

    numbers = malloc(sizeof(node) * lenght);
    threads = malloc(sizeof(pthread_t) * threads_available);

    //populacja tablicy kandydatow
    for(int i = 0; i < lenght; i++)
    {
        numbers[i].num = i;
        numbers[i].deleted = FALSE;
    }

    //1 nie jest liczba pierwsza 
    numbers[1].deleted = TRUE;

    //zaczynam mierzyc czas dla czesci wielowatkowej
	struct timeval start, end;
    gettimeofday(&start, NULL);

    printf("Obliczam liczby pierwsze...\n");
    drawProgress(0);

    for(int i = 2; i < sqrt(lenght); i++)
    {
        //jesli natrafilismy na liczbe wykreslona to idziemy dalej
        if(numbers[i].deleted == TRUE) continue;

        int start = 0;
        //stworz thready i przypisz im dany zakres z tablicy kandydatow
        for(int j = 0; j < threads_available; j++)
        {
            thread_data *data;

            data = malloc(sizeof(thread_data));
            data->numbers = numbers;
            data->numbers_deleted = 0;
            data->div = i;
            data->start = start;
            data->stop = start + lenght_div - 1;
            if(j == threads_available - 1) data->stop += lenght_odd;
            pthread_create(&threads[j], NULL, calculatePrime, (void*)data);

            start += lenght_div;
        }

        for(int j = 0; j < threads_available; j++)
        {
            thread_data *temp;
            pthread_join(threads[j], (void*)&temp);
            deleted_sum += temp->numbers_deleted;
            free(temp);
        }

        drawProgress(i * 100 / sqrt(lenght));
    }

    drawProgress(100);
    gettimeofday(&end, NULL);
    printf("\nWyznaczono liczby pierwsze w %fs. Wykreslono %d liczb.\nZapisywanie do pliku %s...\n", calculateTimeDiff(&start, &end), deleted_sum, file_name);

    //zapisywanie do pliku
    FILE *fp;
    fp = fopen(file_name, "w");

    for(int i = 0; i < lenght; i++)
    {
        if(numbers[i].deleted == FALSE) 
        {
            fprintf(fp, "%d ", numbers[i].num);
        }
    }

    printf("Zakonczono zapisywanie.\n");

    fclose(fp);
    free(numbers);
    free(threads);

    return 0;
}