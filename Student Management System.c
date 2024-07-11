#include<stdio.h>
#include<stdlib.h>
#include<string.h>

void insert_record();
void display_record();
void search_record();
void del_record();
void update_record();
void sort_record();

struct student
{
    int roll, marks;
    char name[20], sec[10];
    float grade;
}s;

int main()
{
    int choice = 0;
    while(choice != 7)
    {
        printf("\n-------------------------------------------");
        printf("\n  Welcome to Student Management System ");
        printf("\n-------------------------------------------");
        printf("\n 1. Insert a record ");        
        printf("\n 2. Display a record ");
        printf("\n 3. Search a record ");
        printf("\n 4. Delete a record ");
        printf("\n 5. Update a record ");
        printf("\n 6. Sort a record ");
        printf("\n 7. Exit");
        printf("\n Enter your choice: ");
        scanf("%d", &choice);
        
        switch(choice)
        {
            case 1:
                    insert_record();
                    break;
            case 2:
                    display_record(); 
                    break;
            case 3:
                    search_record();
                    break;
            case 4:
                    del_record();
                    break;
            case 5:
                    update_record();
                    break;
            case 6:
                    sort_record();
                    break;
            case 7: 
                    printf("\n Task is completed ");
                    exit(0);
                    break;      
            default:
                    printf("\n Invalid Input");                                                                                                                     
        }
    }
    return 0;
}

void insert_record()
{
    FILE* fp;
  
    fp = fopen("student.txt", "ab+");
    if(fp == NULL)
    { 
        printf("\n ERROR: Cannot open the file ");
        return;    
    }     
    
    printf("\n Previous Stored Data ");
    display_record();
  
    printf("\n\n\t******ENTER NEW STUDENT DATA******\n\n");
    printf("\n\t\t Enter Student Roll Number: ");
    scanf("%d", &s.roll);
    
    // Clear input buffer
    while (getchar() != '\n');

    printf("\n\t\t Enter Student Name: ");
    fgets(s.name, 20, stdin);
    s.name[strcspn(s.name, "\n")] = '\0'; // Remove the trailing newline

    printf("\n\t\t Enter Student Section: ");
    fgets(s.sec, 10, stdin);
    s.sec[strcspn(s.sec, "\n")] = '\0'; // Remove the trailing newline

    printf("\n\t\t Enter Student Total Marks: ");
    scanf("%d", &s.marks);

    printf("\n\t\t Enter Student Grade: ");
    scanf("%f", &s.grade);
  
    fwrite(&s, sizeof(s), 1, fp);
    {
        printf("\n\t\t Student Recorded Inserted Successfully ");
    }
    
    fclose(fp);
    printf("\n\t\t Updated Record ");
    display_record();       
}

void display_record()
{
    FILE* fp;
  
    fp = fopen("student.txt", "rb");
    if(fp == NULL)
    { 
        printf("\n ERROR: Cannot open the file ");
        return;    
    }       
      
    printf("\n\t\t****** STUDENT DETAILS ******");
    printf("\nRollno\tName\tSection\tMarks\tGrade\n\n");
  
    while(fread(&s, sizeof(s), 1, fp) == 1)
    {
        printf("%d\t%s\t%s\t%d\t%f\n", s.roll, s.name, s.sec, s.marks, s.grade);
    }
    fclose(fp);          
}

void search_record()
{
    int ro, flag = 0;    
    FILE* fp; 
    fp = fopen("student.txt", "rb");
    if(fp == NULL)
    { 
        printf("\n ERROR: Cannot open the file ");
        return;    
    }       
    
    printf("\n\n\tEnter student roll no which you need to search: ");
    scanf("%d", &ro);
    
    while(fread(&s, sizeof(s), 1, fp) > 0 && flag == 0)
    {
        if(s.roll == ro)
        {
            flag = 1;
            printf("\n Search Successful and Student records are as follows\n\n");
            printf("%d\t%s\t%s\t%d\t%f\n", s.roll, s.name, s.sec, s.marks, s.grade);    
        }      
    }      
    if(flag == 0)
    {
        printf("\n\n\tNo Record Found ");   
    }
    fclose(fp);       
}

void del_record()
{
    int roll;
    unsigned flag = 0;
    FILE* fp, *ft;
    fp = fopen("student.txt", "rb");
    if(fp == NULL)
    { 
        printf("\n ERROR: Cannot open the file ");
        return;    
    }
    
    printf("\n Previous Stored data ");
    display_record();
    printf("\n\n\tEnter student roll number which you want to delete: ");
    scanf("%d", &roll);
    ft = fopen("student1.txt", "wb");
    while(fread(&s, sizeof(s), 1, fp) == 1)
    {
         if(s.roll != roll)
         { 
           fwrite(&s, sizeof(s), 1, ft);
         }
         else
         {
           flag = 1;    
         }       
    }
          
    if(flag == 0)
    {
        printf("\n\n\tNo Record Found ");   
    }
    else
    {
        printf("\n Record DELETED Successfully ");
    }

    fclose(fp);
    fclose(ft);
    remove("student.txt");
    rename("student1.txt", "student.txt");
    display_record();    
}

void update_record()
{
    int ro, flag = 0;    
    FILE* fp; 
    fp = fopen("student.txt", "rb+");
    if(fp == NULL)
    { 
        printf("\n ERROR: Cannot open the file ");
        return;    
    }       
    
    printf("\n\n\tEnter student roll no which you need to update: ");
    scanf("%d", &ro);
    
    while(fread(&s, sizeof(s), 1, fp) > 0 && flag == 0)
    {
        if(s.roll == ro)
        {
            flag = 1;
            printf("\n Search Successful and Student records are as follows\n\n");
            printf("%d\t%s\t%s\t%d\t%f\n", s.roll, s.name, s.sec, s.marks, s.grade);    
            
            printf("\n\t\t******Enter New Record******\n\n");
            
            printf("\n\tUpdated Student Roll no: ");
            scanf("%d", &s.roll);
            
            // Clear input buffer
            while (getchar() != '\n');

            printf("\n\tUpdated Student Name: ");
            fgets(s.name, 20, stdin);
            s.name[strcspn(s.name, "\n")] = '\0'; // Remove the trailing newline

            printf("\n\tUpdated Student Section: ");
            fgets(s.sec, 10, stdin);
            s.sec[strcspn(s.sec, "\n")] = '\0'; // Remove the trailing newline

            printf("\n\tUpdated Student Marks: ");
            scanf("%d", &s.marks);
            printf("\n\tUpdated Student Grade: ");
            scanf("%f", &s.grade);
            
            fseek(fp, -(long)sizeof(s), SEEK_CUR);
            fwrite(&s, sizeof(s), 1, fp);
            
            printf("\n Record Updated Successfully. Check the display ");
        }
    }  
    
    if(flag == 0)
    {
        printf("\n\tERROR: Something went wrong ");  
    }      
    fclose(fp);     
}

void sort_record()
{
    struct student temp;
    struct student arr[50];
    
    int i, j, k = 0;
    FILE* fp;
    fp = fopen("student.txt", "rb");
    
    if(fp == NULL)
    { 
        printf("\n ERROR: Cannot open the file ");
        return;    
    }
    i = 0;
        
    while(fread(&arr[i], sizeof(arr[i]), 1, fp) == 1)
    {
        i++, k++;   
    }          
    
    for(i = 0; i < k; i++)
    {
        for(j = 0; j < k - i - 1; j++)
        {
            if(arr[j].roll > arr[j+1].roll)
            {
                temp = arr[j];
                arr[j] = arr[j+1];
                arr[j+1] = temp;  
            }          
        }      
    }
    
    printf("\n Sorted Student Details are \n");
    for(i = 0; i < k; i++)
    {
         printf("%d\t%s\t%s\t%d\t%f\n", arr[i].roll, arr[i].name, arr[i].sec, arr[i].marks, arr[i].grade);  
    }
    
    fclose(fp);            
}
