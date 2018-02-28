#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXSIZE 256

int dp[MAXSIZE][MAXSIZE];

int pal(char *str, int i, int j) {
	int temp,k=0;
	/*if((j+1)%2!=0){
		temp = (j+1)/2 +1;
	}else{
		temp = (j+1)/2;
	}*/
	while(i <= j){
		if(str[i]>='A' && str[i]<='Z'){
			 str[i] = str[i] + 32;
		}
		if(str[j]>='A' && str[j]<='Z'){
			 str[j] = str[j] + 32;
		}
		if(str[i]!=str[j]){
			return 0;
		}else{
			i++;
			j--;
			k++;
		}
	}
	return 1;
}

int sub(char *str, int i, int j) {
	// caso base
	if (j-i < 2) {
		return 0;
	}
	// si ya calculé este estado
	if (dp[i][j] != -1) return dp[i][j];

	int count=0;
	if(pal(str, i, j)){
		int it;
		printf("\tPalindromo encontrado en [%d, %d]: ", i, j);
		for(it=i;it<=j;it++){
			printf("%c", str[it]);
		}
		printf("\n");
		count++;
	}
	count += sub(str,i+1,j)+sub(str,i,j-1)-sub(str,i+1,j-1);

	return dp[i][j] = count;
}
/*
int main(){
	char str[MAXSIZE];
	scanf("%s", str);
	int i = 0, j = strlen(str)-1;
	//printf("%d\n", pal(i,j));
	memset(dp, -1, sizeof(dp));
	printf("Numero de palindromos: %d\n", sub(str, i, j));
}
*/