#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>

#define MAX_PATH_LEN 512

#define __DEBUG

#ifdef __DEBUG
void debug_info (const char *file, const char *function, const int line)
{
        fprintf(stderr, "DEBUG. ERROR PLACE: File=\"%s\", Function=\"%s\", Line=\"%d\"\n", file, function, line);
}

#define ERR_MSG(DBG_MSG) { \
        perror(DBG_MSG); \
        debug_info(__FILE__, __FUNCTION__, __LINE__); \
}

#else                   

#define ERR_MSG(DBG_MSG) { \
        perror(DBG_MSG); \
}

#endif

char *getPermissionsString(char *name,char *p)
{
	struct stat fileMetadata;
	stat(name, &fileMetadata);
	if (fileMetadata.st_mode & S_IRUSR)     
        strcat(p,"r");
	else    
       strcat(p,"-");
    
	if (fileMetadata.st_mode & S_IWUSR)        
        strcat(p,"w");
	else    
        strcat(p,"-") ;   
    
	if (fileMetadata.st_mode & S_IXUSR)        
        strcat(p,"x");
	else    
        strcat(p,"-");
        
	if (fileMetadata.st_mode & S_IRGRP)        
        strcat(p,"r");
	else    
        strcat(p,"-")  ;  
    
	if (fileMetadata.st_mode & S_IWGRP)        
        strcat(p,"w");
	else    
        strcat(p,"-");   
    
	if (fileMetadata.st_mode & S_IXGRP)        
        strcat(p,"x");
	else    
        strcat(p,"-");
        
	if (fileMetadata.st_mode & S_IROTH)        
        strcat(p,"r");
	else    
        strcat(p,"-");   
    
	if (fileMetadata.st_mode & S_IWOTH)        
        strcat(p,"w");
	else    
        strcat(p,"-");    
    
	if (fileMetadata.st_mode & S_IXOTH)        
        strcat(p,"x");
	else    
        strcat(p,"-");
	return p;
}

void listDir(char *dirName,char *ends,char *permissions,int recursive)
{
    DIR* dir;
    struct dirent *dirEntry;
    struct stat inode;
    char name[MAX_PATH_LEN];

    dir = opendir(dirName);
    if (dir == 0) {
        ERR_MSG ("Error opening directory");
        exit(4);
    }

    while ((dirEntry=readdir(dir)) != 0) {
        snprintf(name, MAX_PATH_LEN, "%s/%s",dirName,dirEntry->d_name);
        lstat (name, &inode);
		char *p=(char*)malloc(sizeof(char)*10);
		if((ends==NULL||strcmp(name+(strlen(name)-strlen(ends)),ends)==0)&&(permissions==NULL||strcmp(permissions,getPermissionsString(name,p))==0)&&strcmp(dirEntry->d_name,".")!=0 && strcmp(dirEntry->d_name,"..")!=0)
		{
			printf(" %s\n",name);
			if (S_ISDIR(inode.st_mode)&&recursive==1)
			{	
				listDir(name,ends,permissions,1);
			}
		}  
		free(p);
    }
    closedir(dir);
}

void listContents(char *path,char *ends,char *permissions,int recursive)
{
	struct stat fileMetadata;
	if (stat(path, &fileMetadata) < 0) { 
        ERR_MSG("ERROR \ninvalid directory path");
        exit(2);
    }
    
    if (S_ISDIR(fileMetadata.st_mode)) { 
		printf("SUCCESS\n");
        listDir(path,ends,permissions,recursive);
    } else {
        printf("%s ERROR \nis not a directory!\n",path);
        exit(3);
    }
}

void parse(char *path)
{
	int fd ;
	if((fd = open(path,O_RDONLY))<0)
		printf("ERROR opening the file");
	char name[12];
	int type = 0;
	int offset = 0;
	int size = 0;
	char magic;
	int header_size = 0;
	int nrSectiuni = 0;
	int version = 0;
	
	lseek(fd, -4, SEEK_END);
	read(fd, &header_size, 2);

	read(fd, &magic, 1);
	if (magic!='g')
	{
		printf("ERROR\nwrong magic\n");
		return;
	}
	read(fd, &magic, 1);
	if (magic!='o')
	{
		printf("ERROR\nwrong magic\n");
		return;
	}

	lseek(fd, -header_size, SEEK_CUR);
	read(fd, &version, 2);

	if (version<16 || version>70)
	{
		printf("ERROR\nwrong version\n");
		return;
	}

	read(fd, &nrSectiuni, 1);
	if (nrSectiuni <5 || nrSectiuni >13)
	{
		printf("ERROR\nwrong sect_nr\n");
		return;
	}


	for (int i = 0;i<nrSectiuni;i++)
	{
		read(fd, &name, 12);
		read(fd, &type, 1);
		if (type != 11 && type != 15 && type != 17 )
		{
			printf("ERROR\nwrong sect_types\n");
			return;
		}
		read(fd, &offset, 4);
		read(fd, &size, 4);
	}


	read(fd, &header_size, 2);
	read(fd, &magic, 2);

	lseek(fd, -header_size, SEEK_CUR);
	lseek(fd, 3, SEEK_CUR);

	printf("SUCCESS\n");
	printf("version=%d\n", version);
	printf("nr_sections=%d\n", nrSectiuni);

	for (int i = 0;i<nrSectiuni;i++)
	{
		read(fd, &name, 12);
		name[12]='\0';
		read(fd, &type, 1);
		read(fd, &offset, 4);
		read(fd, &size, 4);
		printf("section%d: %s %d %d\n", i + 1, name, type, size);
	}

}

void extract(char *path,int section,int line)
{
	int fd ;
	if((fd = open(path,O_RDONLY))<0)
		printf("ERROR invalid file");
	char name[12];
	int type = 0;
	int offset = 0;
	int size = 0;
	char magic;
	int header_size = 0;
	int nrSectiuni = 0;
	int version = 0;
	
	lseek(fd, -4, SEEK_END);
	read(fd, &header_size, 2);

	read(fd, &magic, 1);
	if (magic!='g')
	{
		return;
	}
	read(fd, &magic, 1);
	if (magic!='o')
	{
		return;
	}

	lseek(fd, -header_size, SEEK_CUR);
	read(fd, &version, 2);

	if (version<16 || version>70)
	{
		return;
	}

	read(fd, &nrSectiuni, 1);
	if (nrSectiuni <5 || nrSectiuni >13)
	{
		return;
	}
	if(section>nrSectiuni)
	{
		printf("ERROR\nwerong section");
	}

	for (int i = 0;i<section;i++)
	{
		read(fd, &name, 12);
		read(fd, &type, 1);
		read(fd, &offset, 4);
		read(fd, &size, 4);
	}
	
	int position=0,ct=0;
	char buf;
	printf("SUCCESS\n");
	lseek(fd,offset,SEEK_SET);
	while(size>0)
	{
		size--;
		read(fd,&buf,1);
		position++;
		if(buf=='\n')
		{
			ct++;
			if(ct==line)
			{
				if(position<100000)
				{
					int i=0;
					while(i<position)
					{
						lseek(fd,-2,SEEK_CUR);
						read(fd,&buf,1);
						printf("%c",buf);
						i++;
					}
				}	
				else
				{
					for(int j=0;j<position;j+=100000)
					{
						int i=j;
						while(i<(j+100000)&&i<position)
						{
							lseek(fd,-2,SEEK_CUR);
							read(fd,&buf,1);
							printf("%c",buf);
							i++;
						}
					}
				}
				printf("\n");
				return;
			}
			position=0;
		}
	}
	printf("ERROR\nwrong line\n");
}

int checkSF(char *path)
{
	int fd ;
	if((fd = open(path,O_RDONLY))<0)
		printf("ERROR opening the file");
	char name[12];
	int type = 0;
	int offset = 0;
	int size = 0;
	char magic;
	int header_size = 0;
	int nrSectiuni = 0;
	int version = 0;
	
	lseek(fd, -4, SEEK_END);
	read(fd, &header_size, 2);

	read(fd, &magic, 1);
	if (magic!='g')
	{
		return 0;
	}
	read(fd, &magic, 1);
	if (magic!='o')
	{
		return 0;
	}

	lseek(fd, -header_size, SEEK_CUR);
	read(fd, &version, 2);

	if (version<16 || version>70)
	{
		return 0;
	}

	read(fd, &nrSectiuni, 1);
	if (nrSectiuni <5 || nrSectiuni >13)
	{
		return 0;
	}

	int o[nrSectiuni],s[nrSectiuni];
	for (int i = 0;i<nrSectiuni;i++)
	{
		read(fd, &name, 12);
		read(fd, &type, 1);
		if (type != 11 && type != 15 && type != 17 )
		{
			return 0;
		}
		read(fd, &offset, 4);
		o[i]=offset;
		read(fd, &size, 4);
		s[i]=size;
	}
	int min16=0, ct;
	
	for(int i=0;i<nrSectiuni;i++)
	{
		ct=1;
		lseek(fd,o[i],SEEK_SET);
		char buf;
		while(s[i]>=0)
		{
			s[i]--;
			read(fd,&buf,1);
			if(buf=='\n')
			{
				ct++;
			}
		}
		if(ct==16)
			min16++;
	}
	if(min16<3)
	{
		return 0;
	}
	return 1;
}

void listSF(char *dirName)
{
    DIR* dir;
    struct dirent *dirEntry;
    struct stat inode;
    char name[MAX_PATH_LEN];

    dir = opendir(dirName);
    if (dir == 0) {
        ERR_MSG ("Error opening directory");
        exit(3);
    }

    while ((dirEntry=readdir(dir)) != 0) {
        snprintf(name, MAX_PATH_LEN, "%s/%s",dirName,dirEntry->d_name);
        lstat (name, &inode);
		if(strcmp(dirEntry->d_name,".")!=0 && strcmp(dirEntry->d_name,"..")!=0)
		{
			if(S_ISREG(inode.st_mode)>0&&checkSF(name)==1)
			{
				printf("%s\n",name);
			}  
			if(S_ISDIR(inode.st_mode)>0)
			{
				listSF(name);
			}
		}
    }
    closedir(dir);
}

int main(int argc, char **argv)
{
    if(argc >= 2)
	{
        if(strcmp(argv[1], "variant") == 0)
		{
            printf("12826\n");
        }
		if(strcmp(argv[1],"list")==0)
		{
			if(argc==3)
			{
				listContents(argv[2]+5,NULL,NULL,0);
			}
			else
			{
				if(argc==4)
				{
					if(strstr(argv[3],"path=")!=NULL)
					{
						if(strcmp(argv[2],"recursive")==0)
						{
							listContents(argv[3]+5,NULL,NULL,1);
						}
						else
						{
							if(strstr(argv[2],"name_ends_with=")!=NULL)
							{
								listContents(argv[3]+5,argv[2]+15,NULL,0);
							}
							else
							{
								if(strstr(argv[2],"permissions=")!=NULL)
								{
									listContents(argv[3]+5,NULL,argv[2]+12,0);
								}
							}
						}
							
					}
					else
					{
							if(strstr(argv[2],"path=")!=NULL)
							{
								if(strcmp(argv[3],"recursive")==0)
								{
									listContents(argv[2]+5,NULL,NULL,1);
								}
								else
								{
									if(strstr(argv[3],"name_ends_with=")!=NULL)
									{
										listContents(argv[2]+5,argv[3]+15,NULL,0);
									}
									else
									{
										if(strstr(argv[3],"permissions=")!=NULL)
										{
											listContents(argv[2]+5,NULL,argv[3]+12,0);
										}
									}
								}
							}
						
					}
				}
				else
				{
					if(argc==5)
					{
						if(strstr(argv[2],"path=")!=NULL)
						{
							if(strcmp(argv[3],"recursive")==0)
							{
								if(strstr(argv[4],"name_ends_with=")!=NULL)
								{
									listContents(argv[2]+5,argv[4]+15,NULL,1);
								}
								else
								{
									if(strstr(argv[4],"permissions=")!=NULL)
									{
										listContents(argv[2]+5,NULL,argv[4]+12,1);
									}
								}
							}
							else
							{
								if(strstr(argv[3],"name_ends_with=")!=NULL)
								{
									if(strstr(argv[4],"permissions=")!=NULL)
									{
										listContents(argv[2]+5,argv[3]+15,argv[4]+12,0);
									}
									else
									{
										if(strcmp(argv[3],"recursive")==0)
										{
											listContents(argv[2]+5,argv[3]+15,NULL,1);
										}
									}
								}
								else
								{
									if(strstr(argv[3],"permissions=")!=NULL)
									{
										if(strstr(argv[4],"name_ends_with=")!=NULL)
										{
											listContents(argv[2]+5,argv[4]+15,argv[3]+12,0);
										}
										else
										{
											if(strcmp(argv[3],"recursive")==0)
											{
												listContents(argv[2]+5,NULL,argv[3]+15,1);
											}
										}
									}
								}
								
							}
						}
						else
						{
							if(strstr(argv[3],"path=")!=NULL)
							{
								if(strcmp(argv[2],"recursive")==0)
								{
									if(strstr(argv[4],"name_ends_with=")!=NULL)
									{
										listContents(argv[3]+5,argv[4]+15,NULL,1);
									}
									else
									{
										if(strstr(argv[4],"permissions=")!=NULL)
										{
											listContents(argv[3]+5,NULL,argv[4]+12,1);
										}
									}
								}
								else
								{
									if(strstr(argv[2],"name_ends_with=")!=NULL)
									{
											if(strcmp(argv[4],"recursive")==0)
											{
												listContents(argv[3]+5,argv[2]+15,NULL,1);
											}
									}
									else
									{
										if(strstr(argv[2],"permissions=")!=NULL)
										{
											if(strcmp(argv[4],"recursive")==0)
												{
													listContents(argv[2]+5,NULL,argv[3]+12,1);
												}
										}
									}
								}
							}
							else
							{
								if(strstr(argv[4],"path=")!=NULL)
								{
									if(strcmp(argv[2],"recursive")==0)
									{
										if(strstr(argv[3],"name_ends_with=")!=NULL)
										{
											listContents(argv[4]+5,argv[3]+15,NULL,1);
										}
										else
										{
											if(strstr(argv[3],"permissions=")!=NULL)
											{
												listContents(argv[4]+5,NULL,argv[3]+12,1);
											}
										}
									}
									else
									{
										if(strstr(argv[2],"name_ends_with=")!=NULL)
										{
												if(strcmp(argv[3],"recursive")==0)
												{
													listContents(argv[4]+5,argv[2]+15,NULL,1);
												}
										}
										else
										{
											if(strstr(argv[2],"permissions=")!=NULL)
											{
												if(strcmp(argv[3],"recursive")==0)
													{
														listContents(argv[4]+5,NULL,argv[2]+12,1);
													}
											}
										}
									}
								}
							}	
						}
					}
				}
			}
		}
		if(strcmp(argv[1],"parse")==0)
		{
			if(argc==3)
				parse(argv[2]+5);
			else
				printf("Path not specified");
		}
		if(strcmp(argv[1],"extract")==0)
		{
			if(argc==5)
			{
				extract(argv[2]+5,atoi(argv[3]+8),atoi(argv[4]+5));
			}
			else
			{
				printf("There must be 3 parameters!");
			}
			
		}
		if(strcmp(argv[1],"findall")==0)
		{
			struct stat fileMetadata;
			if (stat(argv[2]+5, &fileMetadata) < 0) { 
				ERR_MSG("ERROR \ninvalid directory path");
				exit(2);
			}
			printf("SUCCESS\n");
			listSF(argv[2]+5);
		}
	}
	return 0;
}