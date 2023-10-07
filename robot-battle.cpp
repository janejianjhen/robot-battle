#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int sizeX, sizeY, pointA, pointB, original_point, accumulated = -1000000, direct = -1, **on_path, bomb_A = 0, bomb_B = 0, distance;
char chessboard[21][21], player, bot;
int accu_deep = 100;
int accu_time[4] = {0, 0, 0, 0};
int rival_deep[4] = {0, 0, 0, 0};
int accu_score[4] = {1000000, 1000000, 1000000, 1000000};

void rivalS(int iA, int jA, int iB, int jB, int A_accu, int deep); /*player side*/
void rivalE(int iA, int jA, int iB, int jB, int A_accu, int deep); /*bot side*/

int add(int i, int j, int accu_point){
	if(chessboard[i][j] == 'm'){
		return accu_point+1;
	}else if(chessboard[i][j] == 'n'){
		return accu_point-1;
	}else if(chessboard[i][j] == 's'){
		return accu_point*2;
	}else if(chessboard[i][j] == 't'){
		return accu_point/2;
	}
	return accu_point;
}

void compute(int accu_point, int deep){
	if(accu_point > original_point){ /*smaller deep to add points faster*/
		if(accu_point > accumulated || accu_point == accumulated && accu_deep > deep){
			accumulated = accu_point;
			for(int i=0 ; i<4 ; ++i){
				accu_time[i] = 0;
			}
			accu_time[direct] = 1;
			accu_deep = deep;
		}else if(accu_point == accumulated && accu_deep == deep){
			accu_time[direct] += 1;
		}
	}else{ /*larger deep to lose points slower*/
		if(accu_point > accumulated || accu_point == accumulated && accu_deep < deep){
                        accumulated = accu_point;
                        for(int i=0 ; i<4 ; ++i){
                                accu_time[i] = 0;
                        }
                        accu_time[direct] = 1;
                        accu_deep = deep;
                }else if(accu_point == accumulated && accu_deep == deep){
                        accu_time[direct] += 1;
                }
	}
}

void endingCount(int A_accu, int deep){ /*Settling scores for each path*/
	if(distance > 2){ /*general calculation*/
		compute(A_accu, deep);
	}else{ /*minimax*/
		if(direct != -1 && A_accu >= original_point && A_accu < accu_score[direct]){
			accu_score[direct] = A_accu;
			accu_time[direct] = 1;
			rival_deep[direct] = deep;
		}else if(direct != -1 && A_accu == accu_score[direct]){
			if(A_accu > original_point){
				if(deep < rival_deep[direct]){
					rival_deep[direct] = deep;
				}else if(deep == rival_deep[direct]){
					accu_time[direct] += 1;
				}
			}else{
				if(deep > rival_deep[direct]){
                                        rival_deep[direct] = deep;
                                }else if(deep == rival_deep[direct]){
                                        accu_time[direct] += 1;
                                }
			}
		}
	}
}

void analysis(int x, int y, int point, int round){ /*increase the utility of super star and avoid super toxic star*/
	for(int i=0 ; i<sizeX ; ++i){
		for(int j=0 ; j<sizeY ; j++){
			if(chessboard[i][j] == 's' && point < 2 && (abs(x-i)+abs(y-j)) > 6 && round < 200){
				chessboard[i][j] = 'k';
			}else if(chessboard[i][j] == 't' && point > 2){
				chessboard[i][j] = 'k';
			}
		}
	}
}

int decide_DFS(int x, int y){ /*find the output direction whose score is determined by maximum value*/
	int ans = 0, max = -10000000;
	for(int i=0 ; i<4 ; ++i){
		if(accu_time[i] > max){
			ans = i;
			max = accu_time[i];
		}
	}
	if(max == 0){
		if(y+1 < sizeY && chessboard[x][y+1] != 'x' && chessboard[x][y+1] != bot){
                        ans = 0;
                }else if(x+1 < sizeX && chessboard[x+1][y] != 'x' && chessboard[x+1][y] != bot){
                        ans = 1;
                }else if(y-1 >= 0 && chessboard[x][y-1] != 'x' && chessboard[x][y-1] != bot){
                        ans = 2;
                }else if(x-1 >= 0 && chessboard[x-1][y] != 'x' && chessboard[x-1][y] != bot){
                        ans = 3;
                }else{
                        ans = 0;
                }
	}
	return ans;
}

int check_corner(int x_next, int y_next, int x_rival, int y_rival){ /*avoid going into dead ends*/
	int check = 0;
	if(y_next+1 >= sizeY || chessboard[x_next][y_next+1] == 'x' || (x_next == x_rival && y_next+1 == y_rival)){++check;}
	if(x_next+1 >= sizeX || chessboard[x_next+1][y_next] == 'x' || (x_next+1 == x_rival && y_next == y_rival)){++check;}
	if(y_next-1 < 0 || chessboard[x_next][y_next-1] == 'x' || (x_next == x_rival && y_next-1 == y_rival)){++check;}
	if(x_next-1 < 0 || chessboard[x_next-1][y_next] == 'x' || (x_next-1 == x_rival && y_next == y_rival)){++check;}
	return check;
}

/*find the output direction whose score is determined by minimax*/
int decide_rival(int x, int y, int x_rival, int y_rival){ /*x, y represent player's position*/
	int ans = -1, max = -10000000, deep = 0;
	for(int i=0 ; i<4 ; ++i){
		if(accu_score[i] > max && accu_score[i] != 1000000){
			ans = i;
			max = accu_score[i];
			deep = rival_deep[i];
		}else if(accu_score[i] == max){
			if(accu_score[i] > original_point){
				if(rival_deep[i] < deep){
					ans = i;
					deep = rival_deep[i];
				}else if(rival_deep[i] == deep && accu_time[i] > accu_time[ans]){
					ans = i;
				}
			}else{
				if(rival_deep[i] > deep){
                                        ans = i;
                                        deep = rival_deep[i];
                                }else if(rival_deep[i] == deep && accu_time[i] > accu_time[ans]){
                                        ans = i;
                                }
			}
		}
	}
	if(ans != -1 && deep == 2){ /*check the corner*/
		int check = 0, x_next, y_next;
		if(ans == 0){x_next = x; y_next = y+1;}
		else if(ans == 1){x_next = x+1; y_next = y;}
		else if(ans == 2){x_next = x; y_next = y-1;}
		else if(ans == 3){x_next = x-1; y_next = y;}
		if(x == x_rival && y == y_rival+1){check = check_corner(x_next, y_next, x_rival, y_rival+1);}
		else if(x == x_rival+1 && y == y_rival){check = check_corner(x_next, y_next, x_rival+1, y_rival);}
		else if(x == x_rival && y == y_rival-1){check = check_corner(x_next, y_next, x_rival, y_rival-1);}
		else if(x == x_rival-1 && y == y_rival){check = check_corner(x_next, y_next, x_rival-1, y_rival);}
		if(check == 4){
			chessboard[x_next][y_next] = 'x';
			ans = -1;
		}
	}
	if(ans == -1){ /*escape from corner or find one direction when no answer*/
		if(y+1 < sizeY && chessboard[x][y+1] != 'x' && chessboard[x][y+1] != bot){
			ans = 0;
		}else if(x+1 < sizeX && chessboard[x+1][y] != 'x' && chessboard[x+1][y] != bot){
			ans = 1;
		}else if(y-1 >= 0 && chessboard[x][y-1] != 'x' && chessboard[x][y-1] != bot){
			ans = 2;
		}else if(x-1 >= 0 && chessboard[x-1][y] != 'x' && chessboard[x-1][y] != bot){
			ans = 3;
		}else{
			ans = 0;
		}
	}
	return ans;
}

void rivalS(int iA, int jA, int iB, int jB, int A_accu, int deep){ /*player side*/
	int check = 0;
	if(++deep < 10 && (chessboard[iA][jA] != 'b' || chessboard[iA][jA] == 'b' && bomb_A == 3)){
		if(on_path[iA][jA] != 1){
			A_accu = add(iA, jA, A_accu);
		}
		on_path[iA][jA] = 1;
		if(jA+1<sizeY && on_path[iA][jA+1]==0 && chessboard[iA][jA+1]!='x' && chessboard[iA][jA+1]!='k' && (iA!=iB || iA==iB && jA+1!=jB)){
			check = 1;
			if(deep == 1){direct = 0;}
			rivalE(iA, jA+1, iB, jB, A_accu, deep);
		}
		if(iA+1<sizeX && on_path[iA+1][jA]==0 && chessboard[iA+1][jA]!='x' && chessboard[iA+1][jA]!='k' && (jA!=jB || jA==jB && iA+1!=iB)){
			check = 1;
			if(deep == 1){direct = 1;}
			rivalE(iA+1, jA, iB, jB, A_accu, deep);
		}
		if(jA-1>=0 && on_path[iA][jA-1]==0 && chessboard[iA][jA-1]!='x' && chessboard[iA][jA-1]!='k' && (iA!=iB || iA==iB && jA-1!=jB)){
			check = 1;
			if(deep == 1){direct = 2;}
			rivalE(iA, jA-1, iB, jB, A_accu, deep);
		}
		if(iA-1>=0 && on_path[iA-1][jA]==0 && chessboard[iA-1][jA]!='x' && chessboard[iA-1][jA]!='k' && (jA!=jB || jA==jB && iA-1!=iB)){
			check = 1;
			if(deep == 1){direct = 3;}
			rivalE(iA-1, jA, iB, jB, A_accu, deep);
		}
		if(check == 0){
			endingCount(A_accu, deep);
		}
	}else if(deep < 10 && chessboard[iA][jA] == 'b' && bomb_A < 3){
		on_path[iA][jA] = 1;
		++bomb_A;
		rivalE(iA, jA, iB, jB, A_accu, deep);
		on_path[iA][jA] = 0;
		bomb_A = 0;
	}else{
		endingCount(A_accu, deep);
	}
	if(chessboard[iA][jA] != 'b'){on_path[iA][jA] = 0;}
}

void rivalE(int iA, int jA, int iB, int jB, int A_accu, int deep){ /*bot side*/
	int check = 0;
	if(deep < 10 && (chessboard[iB][jB] != 'b' || chessboard[iB][jB] != 'b' && bomb_B == 3)){
		on_path[iB][jB] = 1;
		if(jB+1<sizeY && on_path[iB][jB+1]==0 && chessboard[iB][jB+1]!='x' && chessboard[iB][jB+1]!='k' && (iA!=iB || iA==iB && jA!=jB+1)){
			check = 1;
			rivalS(iA, jA, iB, jB+1, A_accu, deep);
		}
		if(iB+1<sizeX && on_path[iB+1][jB]==0 && chessboard[iB+1][jB]!='x' && chessboard[iB+1][jB]!='k' && (jA!=jB || jA==jB && iA!=iB+1)){
			check = 1;
			rivalS(iA, jA, iB+1, jB, A_accu, deep);
		}
		if(jB-1>=0 && on_path[iB][jB-1]==0 && chessboard[iB][jB-1]!='x' && chessboard[iB][jB-1]!='k' && (iA!=iB || iA==iB && jA!=jB-1)){
			check = 1;
			rivalS(iA, jA, iB, jB-1, A_accu, deep);
		}
		if(iB-1>=0 && on_path[iB-1][jB]==0 && chessboard[iB-1][jB]!='x' && chessboard[iB-1][jB]!='k' && (jA!=jB || jA==jB && iA!=iB-1)){
			check = 1;
			rivalS(iA, jA, iB-1, jB, A_accu, deep);
		}
		if(check == 0){
			rivalS(iA, jA, iB, jB, A_accu, deep);
		}
	}else if(deep < 10 && chessboard[iB][jB] == 'b' && bomb_B < 3){
		on_path[iB][jB] = 1;
		++bomb_B;
		rivalS(iA, jA, iB, jB, A_accu, deep);
		on_path[iB][jB] = 0;
		bomb_B = 0;
	}
	if(chessboard[iB][jB] != 'b'){on_path[iB][jB] = 0;}
}

void DFS(int i, int j, int accu_point, int deep){ /*not consider opponent*/
	int check = 0;
	if(chessboard[i][j] == 'b'){
		deep += 2;
	}
	if(++deep < 12){
		on_path[i][j] = 1;
		accu_point = add(i, j, accu_point);
		if(j+1<sizeY && on_path[i][j+1]==0 && chessboard[i][j+1]!='x' && chessboard[i][j+1]!='k' && chessboard[i][j+1]!=bot){
			check = 1;
			if(deep == 1){direct = 0;}
			DFS(i, j+1, accu_point, deep);
		}
		if(i+1<sizeX && on_path[i+1][j]==0 && chessboard[i+1][j]!='x' && chessboard[i+1][j]!='k' && chessboard[i+1][j]!=bot){
			check = 1;
			if(deep == 1){direct = 1;}
			DFS(i+1, j, accu_point, deep);
		}
		if(j-1>=0 && on_path[i][j-1]==0 && chessboard[i][j-1]!='x' && chessboard[i][j-1]!='k' && chessboard[i][j-1]!=bot){
			check = 1;
			if(deep == 1){direct = 2;}
			DFS(i, j-1, accu_point, deep);
		}
		if(i-1>=0 && on_path[i-1][j]==0 && chessboard[i-1][j]!='x' && chessboard[i-1][j]!='k' && chessboard[i-1][j]!=bot){
			check = 1;
			if(deep == 1){direct = 3;}
			DFS(i-1, j, accu_point, deep);
		}
	}
	if(check == 0){
		compute(accu_point, deep);
	}
	on_path[i][j] = 0;
}

int main(){
	char output[4][6] = {"RIGHT", "DOWN", "LEFT", "UP"};
	int round, A_X, A_Y, B_X, B_Y;
	scanf("%d\n%d%d\n", &round, &sizeX, &sizeY);
	on_path = (int**)malloc(sizeX * sizeof(int*));
	for(int i=0 ; i<sizeX ; ++i){
		for(int j=0 ; j<sizeY ; ++j){
			scanf("%c ", &chessboard[i][j]);
			if(chessboard[i][j] == 'A'){
				A_X = i;
				A_Y = j;
			}else if(chessboard[i][j] == 'B'){
				B_X = i;
				B_Y = j;
			}
		}
		scanf("\n");
		on_path[i] = (int*)calloc(sizeY, sizeof(int));
	}
	scanf("%d\n%d\n%c\n", &pointA, &pointB, &player);
	int answer;
	distance = abs(A_X-B_X)+abs(A_Y-B_Y);
	if(distance <= 2){ /*exception of minimax*/
		if(check_corner(B_X, B_Y, A_X, A_Y) >= 2 || check_corner(A_X, A_Y, B_X, B_Y) >= 2){
			distance = 3;
		}
		if((A_X==B_X||A_Y==B_Y)){
                	if(A_X==B_X && abs(A_Y-B_Y)==2 && (chessboard[A_X][(A_Y+B_Y)/2]=='x'||chessboard[A_X][(A_Y+B_Y)/2]=='t')){
                        	distance = 3;
                	}else if(A_Y==B_Y && abs(A_X-B_X)==2 && (chessboard[(A_X+B_X)/2][A_Y]=='x'||chessboard[(A_X+B_X)/2][A_Y]=='t')){
                        	distance = 3;
                	}
		}
        }
	if(player == 'A'){
		original_point = pointA;
		analysis(B_X, B_Y, pointA, round);
		bot = 'B';
		if(distance >= 15){ /*Regardless of the opponent, use player's maximum score as answer*/
			DFS(A_X, A_Y, pointA, 0);
			answer = decide_DFS(A_X, A_Y);
		}else if(distance < 15 && distance > 2){ /*Consider the opponent, use player's maximum score as answer*/
			rivalS(A_X, A_Y, B_X, B_Y, pointA, 0);
			answer = decide_DFS(A_X, A_Y);
		}else{ /*Consider the opponent, use player's minimax score as answer*/
			rivalS(A_X, A_Y, B_X, B_Y, pointA, 0);
			answer = decide_rival(A_X, A_Y, B_X, B_Y);
		}
	}else{
		original_point = pointB;
                analysis(A_X, A_Y, pointB, round);
                bot = 'A';
                if(distance >= 15){ /*Regardless of the opponent, use player's maximum score as answer*/
                        DFS(B_X, B_Y, pointB, 0);
                        answer = decide_DFS(B_X, B_Y);
                }else if(distance < 15 && distance > 2){ /*Consider the opponent, use player's maximum score as answer*/
                        rivalS(B_X, B_Y, A_X, A_Y, pointB, 0);
                        answer = decide_DFS(B_X, B_Y);
                }else{ /*Consider the opponent, use player's minimax score as answer*/
                        rivalS(B_X, B_Y, A_X, A_Y, pointB, 0);
                        answer = decide_rival(B_X, B_Y, A_X, A_Y);
                }
	}
	printf("%s\n", output[answer]);
	return 0;
}
