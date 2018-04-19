#include "sfp.h"
#include<stdlib.h>
#include<math.h>

sfp int2sfp(int input) {
	// 1. 매개변수 int형 input을 2진수로 변환한다.
	// 2. (-1)^s * M * 2^E 형태로 변환한다.
	// 3. S 에 대하여 
	// 4. E 에 대하여
	// 4-1. (변환된 E)exp 가 62보다 커지면 overflow 처리 ,S가 0 이면 +무한대, S가 1이면 -무한대
	// 4-2. exp를 2진수로 변환하여 sfp에 담는다.
	// 5. M 에 대하여
	// 5-1. (변환된 M) frac이 9bit 초과로 가질 경우
	// 5-2. Round-to-zero mode에 의해 10bit부터는 그냥 drop한다.

	sfp result = 0; // 반환 값
	int target = input;
	int SFParr[16] = { 0, };

	if (input == 0)
		return result;

	// 1. 2진수로 변환
	int TempBinary[31] = { 0, };
	int Binary[31] = { 0, }; // result를 2진수로 변환하여 담는다.
	int idx = 0;
	while (1) {
		TempBinary[idx++] = target % 2;
		target = target / 2;
		if (target == 0)
			break;
	}
	int j = 0;	// j = 비트 갯수를 가짐
	for (int i = idx - 1; i >= 0; i--, j++) {  // 거꾸로 저장된 2진수를 올바르게 Binary에 담는다.
		Binary[j] = TempBinary[i];
	}

	// 3. S에 대하여
	if (input > 0)
		SFParr[0] = 0;
	else
		SFParr[0] = 1;

	// 4. E에 대하여
	int E;
	int Bias = 31;
	int exp;
	E = j - 1;	// j는 비트갯수를 담고 있기에 E를 구할 수 있다.
	exp = E + Bias;

	// 4-1.overflow처리
	int PlusInfinityArr[16] = { 0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0 };
	int MinusInfinityArr[16] = { 1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0 };
	if (exp > 62) {
		if (SFParr[0] == 0) { // 양의 무한대
			for (int i = 0; i < 16; i++) {
				if (PlusInfinityArr[i] == 0) {
					result = result << 1;
				}
				else {
					result = result << 1;
					result++;
				}
			}
		}
		else { // 음의 무한대
			for (int i = 0; i < 16; i++) {
				if (PlusInfinityArr[i] == 0) {
					result = result << 1;
				}
				else {
					result = result << 1;
					result++;
				}
			}
		}
		return result;
	}
		
	// 4.2 exp 2진수 변환 후 SFParr에 담기
	int TempBinary2[6] = { 0, }; // 0<= exp <= 62  
	int idx2 = 0;
	while (1) {
		TempBinary2[idx2++] = exp % 2;
		exp = exp / 2;
		if (exp == 0)
			break;
	}
	for (int i = 5; i >= 0; i--) { // exp를 담는다.
		SFParr[5 - i + 1] = TempBinary2[i];
	}

	// 5. M에 대하여
	// 5.1 Binary배열의 1번요소부터 9번째요소까지 담는다.
	// 5.2 Round-to-zero 이기에 10번째는 자동 drop
	for (int i = 1; i <10; i++) {
		SFParr[i + 6] = Binary[i];
	}

	// 6. SFP배열의 비트를 통해 result의 비트를 조작하여 반환
	for (int i = 0; i < 16; i++) {
		if (SFParr[i] == 0) {
			result = result << 1;
		}
		else { // 1이면
			result = result << 1;
			result++;
		}
	}
	return result;
}


int sfp2int(sfp input) {
	// 1. sfp형 input을 arr배열에 비트단위로 담아준다.
	// 2. 예외 처리
	// 2-1. 양의 무한대일 경우 // TMAX으로 반환
	// 2-2. 음의 무한대일 경우 // TMIN으로 반환
	// 2-3. Nan일 경우		// TMIN으로 반환
	// 3. Sign부 정리
	// 4. Exp부 정리
	// 5. Frac부 정리
	// 6. 10진수 변환
	int result = 0;

	// 1. input을 비트단위로 arr에 저장
	int arr[16] = { 0, };
	int num = 0;
	int j = 0;
	for (int i = 0; i < 16; i++) {
		num = (input & 1 << i) >> i;
		arr[15 - j++] = num;
	}

	//2. 예외 처리
	// 2-1. 양의 무한대일 경우 // TMAX으로 반환
	// 2-2. 음의 무한대일 경우 // TMIN으로 반환
	int PlusInfinityArr[16] = { 0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0 };
	int MinusInfinityArr[16] = { 1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0 };
	int check = 0;
	int check2 = 0;
	for (int i = 0; i < 16; i++) {
		if (arr[i] == PlusInfinityArr[i])
			check++;
		if (arr[i] == MinusInfinityArr[i])
			check2++;
	}
	if (check == 16)
		return 2147483647;
	if (check2 == 16)
		return -2147483648;

	// 2-3. Nan일 경우		// TMIN으로 반환
	int Nan = 0;
	for (int i = 1; i < 7; i++) {
		if (arr[i] == 1)
			Nan++;
	}
	for (int i = 7; i < 16; i++) {
		if (arr[i] == 1 && Nan == 6)	// exp가 전부 1이고 frac이 000...0 이 아니라면. 
			return -2147483648;
	}

	// 3. sign부 화인 후 sign변수를 마지막 결과에 곱해준다.
	int sign = 0;
	if (arr[0] == 1)
		sign = -1;
	else
		sign = 1;

	// 4. Exp부
	int exp = 0;
	int E;
	int Bias = 31;
	for (int j = 6; j >= 1; j--) {
		if (arr[j] == 1) {
			exp += 1 << (6 - j);
		}
	}
	E = exp - Bias;

	// 5. Frac부
	int Frac[10] = { 0, };
	for (int i = 0; i < 9; i++) {
		Frac[i + 1] = arr[i + 7];
	}
	Frac[0] = 1;

	//6. 10진수 변환
	for (int j = E; j >= 0; j--) {
		if (Frac[j] == 1) {
			result += 1 << (E - j);
		}
	}

	return result * sign;

}

sfp float2sfp(float input) {
	// 1. sign부 정리
	// 2. float형 input에 대하여 2진수로 변환한다.

	sfp result = 0;

	float target = input;
	int SFParr[16] = { 0, };

	//1. sign부 정리
	if (target > 0)
		SFParr[0] = 0;
	else
		SFParr[0] = 1;

	// 2. float형 input에 대하여 2진수로 변환한다.
	// 2-1. 소수점 좌측과 우측을 나누어 2진수로 변환한다.
	int Front = 0; // 소수점 좌측
	float Back = 0; // 소수점 우측
	if (target > 0) {
		Front = input;
		Back = input - Front;
	}
	else {
		Front = input * -1;
		Back = (input + Front)*-1;
	}

	int Binary[200] = { 0, }; // input을 2진수로 변환하여 담는다.
	int TempBinary[200] = { 0, };

	// 2-2. Front 부 2진수 변환
	int idx = 0;
	if (Front != 0) {	// 0.xx가 아닐 경우
		while (1) {

			TempBinary[idx++] = Front % 2;
			Front = Front / 2;

			if (Front == 0)
				break;
		}
	}
	int j = 0;	// j = 비트 갯수를 가짐, 소수부 시작부분을 의미하는 인덱스이다.
	for (int i = idx - 1; i >= 0; i--, j++) { // 거꾸로 저장된 2진수를 올바르게 Binary에 담는다.
		Binary[j] = TempBinary[i];
	}

	// 2-3. Back 부 2진수 변환
	//  Binary배열의 j번째 인덱스부터 담는다.
	// 소수부가 0으로 떨어지지 않는다면 비트는 계속해서 나온다. So Binary배열에 200-j 개만큼만 저장한다.
	int Bidx = j;
	int whileBreaker = 0;
	while (whileBreaker + j<199) {
		if (Back * 2 >= 1) {
			Binary[Bidx++] = 1;
			Back = Back * 2 - 1;
		}
		else {
			Binary[Bidx++] = 0;
			Back = Back * 2;
		}
		if (Back == 0)
			break;
		whileBreaker++;
	}

	// 3. exp 부 정리 
	int E = 0;
	int Bias = 31;
	int exp;
	int TempBinary2[6] = { 0, }; // exp를 2진수로 변환한다.
	int idx2 = 0;
	int FracIdx = 7;
	// -1. case 분류
	// 1. 0.xxx로 된 경우
	if (j == 0) { // 0.xxx 로 된 경우  소수점에서 처음 등장하는 1을 찾는다.
		while (1) {
			if (Binary[E++] == 1) {
				exp = E * -1 + Bias;
				break;
			}
		}
		// 예외처리
		// exp < 0 일 경우 값이 매우 작아 표현할 수 없다.  0에 근접한다.
		if (exp < 0)
			return 0;
		while (1) {
			TempBinary2[idx2++] = exp % 2;
			exp = exp / 2;
			if (exp == 0)
				break;
		}
		// exp를 담는다.
		for (int i = 5; i >= 0; i--) {
			SFParr[5 - i + 1] = TempBinary2[i];
		}
		// frac을 담는다.
		for (int i = 0; i < 9; i++) {	// E는 Binary에서 1이 처음 등장하는 인덱스+1이다. 
			SFParr[FracIdx++] = Binary[E++]; // Round-to-zero이기에 Frac에 9bit만 담고 나머지는 drop한다.
		}
	}
	else { // xxx.xxx 로 된 경우
		E = j - 1;
		exp = E + Bias;

		// 예외처리 : overflow처리
		int PlusInfinityArr[16] = { 0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0 };
		int MinusInfinityArr[16] = { 1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0 };
		if (exp > 62) {
			if (SFParr[0] == 0) { // 양의 무한대
				for (int i = 0; i < 16; i++) {
					if (PlusInfinityArr[i] == 0) {
						result = result << 1;
					}
					else {
						result = result << 1;
						result++;
					}
				}
			}
			else { // 음의 무한대
				for (int i = 0; i < 16; i++) {
					if (PlusInfinityArr[i] == 0) {
						result = result << 1;
					}
					else {
						result = result << 1;
						result++;
					}
				}
			}
			return result;
		}

		while (1) {
			TempBinary2[idx2++] = exp % 2;
			exp = exp / 2;
			if (exp == 0)
				break;;
		}
		// exp를 담는다.
		for (int i = idx2 - 1, k = 1; i >= 0; i--, k++) {
			SFParr[k] = TempBinary2[i];
		}
		// frac을 담는다.
		for (int i = 0; i < 9; i++) {
			SFParr[FracIdx++] = Binary[i + 1];
		}
	}

	// 5. result에 담아준다.
	for (int i = 0; i < 16; i++) {
		if (SFParr[i] == 0) {
			result = result << 1;
		}
		else { // 1이면
			result = result << 1;
			result++;
		}

	}

	return result;
}

float sfp2float(sfp input) {
	// 비트 단위로 배열에 담는다
	float result = 0;

	// arr배열에 input의 비트를 담는다.
	int arr[16] = { 0, };
	int num = 0;
	int j = 0;
	for (int i = 0; i < 16; i++) {
		num = (input & 1 << i) >> i;
		arr[15 - j++] = num;
	}

	// 예외 처리 : 무한대
	int PlusInfinityArr[16] = { 0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0 };
	int MinusInfinityArr[16] = { 1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0 };
	int check = 0;
	int check2 = 0;
	for (int i = 0; i < 16; i++) {
		if (arr[i] == PlusInfinityArr[i])
			check++;
		if (arr[i] == MinusInfinityArr[i])
			check2++;
	}
	if (check == 16)
		return 340282346638528859811704183484516925440.00; // FLT_MAX
	if (check2 == 16)
		return -340282346638528859811704183484516925440.00; // FLT_MIN

	//예외처리 :  Nan 
	int Nan = 0;
	for (int i = 1; i < 7; i++) {
		if (arr[i] == 1)
			Nan++;
	}
	for (int i = 7; i < 16; i++) {
		if (arr[i] == 1 && Nan == 6)
			return -340282346638528859811704183484516925440.00;
	}

	// 3. sign부 정리
	// sign부 화인 후 sign변수를 마지막 결과에 곱해준다.
	int sign = 0;
	if (arr[0] == 1)
		sign = -1;
	else
		sign = 1;

	// 4. exp부 정리
	int exp = 0;
	int E;
	int Bias = 31;
	for (int j = 6; j >= 1; j--) {
		if (arr[j] == 1) {
			exp += 1 << (6 - j);
		}
	}
	E = exp - Bias;

	// 5. Frac부 정리
	int Frac[10] = { 0, };
	Frac[0] = 1;

	for (int i = 0; i < 9; i++) {
		Frac[i + 1] = arr[i + 7];
	}

	// 10진수 변환
	// 정수부분
	for (int j = 0; j < 10; j++) {
		if (Frac[j] == 1) {
			result += pow(2, E - j);
		}
	}


	return result * sign;
}

sfp sfp_add(sfp a, sfp b) {
	// Round-to-even을 적용시킨다.
	// 1. a와 b를 (-1)^s * M * 2^E형으로 변환한다.
	// 2. E1과 E2를 비교하여 큰 값으로 통일한다.
	// 3. M1과 M2를 더해준다.
	// 3-1. 더해진 M에 대하여 9번과 10번 비트를 보고 반올림을 결정한다.
	// 3-1. 더한 결과가 exp >62일 경우 overflow
	//4. 더한 결과인 (-1)^s* M * 2^E를 sfp에 저장해준다.

	sfp result;
	int SFParr[16] = { 0, };
	// 1. a에 대하여
	int arr_A[16] = { 0, };
	int num_A = 0;
	int j_A = 0;
	for (int i = 0; i < 16; i++) {
		num_A = (a & 1 << i) >> i;
		arr_A[15 - j_A++] = num_A;
	}
	// 지수부
	int exp_A = 0;
	int E_A;
	int Bias = 31;
	for (int j = 6; j >= 1; j--) {
		if (arr_A[j] == 1) {
			exp_A += 1 << (6 - j);
		}
	}
	E_A = exp_A - Bias;

	// 1. b에 대하여
	int arr_B[40] = { 0, };
	int num_B = 0;
	int j_B = 0;
	for (int i = 0; i < 16; i++) {
		num_B = (b & 1 << i) >> i;
		arr_B[15 - j_B++] = num_B;
	}

	// 지수부
	int exp_B = 0;
	int E_B;
	for (int j = 6; j >= 1; j--) {
		if (arr_B[j] == 1) {
			exp_B += 1 << (6 - j);
		}
	}
	E_B = exp_B - Bias;

	// Special result value  ===========================================
	int PlusInfinityArr[16] = { 0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0 };
	int MinusInfinityArr[16] = { 1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0 };
	int Nan[16] = { 0,1,1,1,1,1,1,0,0,0,0,0,0,1,0,0 };
	int check_A = 0;
	int check2_A = 0;
	int check_B = 0;
	int check2_B = 0;
	for (int i = 0; i < 16; i++) {
		if (arr_A[i] == PlusInfinityArr[i])
			check_A++;
		if (arr_A[i] == MinusInfinityArr[i])
			check2_A++;
	}
	for (int i = 0; i < 16; i++) {
		if (arr_B[i] == PlusInfinityArr[i])
			check_B++;
		if (arr_B[i] == MinusInfinityArr[i])
			check2_B++;
	}
		// +무한대 +무한대인 경우
	if (check_A == 16 && check_B == 16) {
		for (int i = 0; i < 16; i++) {
			if (PlusInfinityArr[i] == 0) {
				result = result << 1;
			}
			else {
				result = result << 1;
				result++;
			}
		}
		return result;
	}
		// +무한대 -무한대인 경우
	if (check_A == 16 && check2_B == 16) {
		for (int i = 0; i < 16; i++) {
			if (Nan[i] == 0) {
				result = result << 1;
			}
			else {
				result = result << 1;
				result++;
			}
		}
		return result;
	}
		// -무한대 +무한대인 경우
	if (check2_A == 16 && check_B == 16) {
		for (int i = 0; i < 16; i++) {
			if (Nan[i] == 0) {
				result = result << 1;
			}
			else {
				result = result << 1;
				result++;
			}
		}
		return result;
	}
		// -무한대 -무한대인 경우
	if (check2_A == 16 && check2_B == 16) {
		for (int i = 0; i < 16; i++) {
			if (MinusInfinityArr[i] == 0) {
				result = result << 1;
			}
			else {
				result = result << 1;
				result++;
			}
		}
		return result;
	}
		// 하나만 +무한대인 경우
	if (check_A == 16 || check_B == 16) {
		for (int i = 0; i < 16; i++) {
			if (PlusInfinityArr[i] == 0) {
				result = result << 1;
			}
			else {
				result = result << 1;
				result++;
			}
		}
		return result;
	}
		// 하나만 -무한대인 경우
	if (check2_A == 16 || check2_B == 16) {
		for (int i = 0; i < 16; i++) {
			if (MinusInfinityArr[i] == 0) {
				result = result << 1;
			}
			else {
				result = result << 1;
				result++;
			}
		}
		return result;
	}
		//예외처리 :  Nan 
	int N_A = 0;
	for (int i = 1; i < 7; i++) {
		if (arr_A[i] == 1)
			N_A++;
	}
	int N_B = 0;
	for (int i = 1; i < 7; i++) {
		if (arr_A[i] == 1)
			N_B++;
	}
	if (N_A == 6 || N_B == 6) {
		for (int i = 0; i < 16; i++) {
			if (Nan[i] == 0) {
				result = result << 1;
			}
			else {
				result = result << 1;
				result++;
			}
		}
		return result;
	}
	//======================================================


	// 2. E1 과  E2를 비교하여 큰 값에 맞춘다.
	int M_A[13] = { 0,1,0,0,0,0,0,0,0,0,0,0,0 };
	for (int i = 0; i < 9; i++) {
		M_A[i + 2] = arr_A[i + 7];
	}
	int M_B[13] = { 0,1,0,0,0,0,0,0,0,0,0,0,0 };
	for (int i = 0; i < 9; i++) {
		M_B[i + 2] = arr_B[i + 7];
	}
	int M_B2[13] = { 0, }; // E가 작은 M의 수정된 M
	int M[13] = { 0, };
	int E;
	int gap;
	int flag = 0;
	if (E_A > E_B) {
		// E1 - E2 만큼 M2를 뒤로 민다.  그 결과를 M에 저장
		gap = E_A - E_B;
		E = E_A;
		for (int i = 1; i<12; i++) {
			if (i + gap > 12)
				break;
			M_B2[i + gap] = M_B[i];
		}
		// M_A와 M_B2를 더하거나 빼서 M에 저장한다.
		if ((arr_A[0] == 0 && arr_B[0] == 0)) { // A와 B 둘다 양수인 경우 더하기
			for (int i = 12; i >= 0; i--) {
				if (flag == 0) {// 직전 연산에서 넘어온 1이 없다면
					if (M_A[i] == 0 && M_B2[i] == 0)
						M[i] = 0;
					else if (M_A[i] == 1 && M_B2[i] == 1) {
						M[i] = 0;
						flag = 1;
					}
					else { // 둘 중 하나만 1인경우
						M[i] = 1;
					}
				} 
				else {
					if (M_A[i] == 0 && M_B2[i] == 0) {
						M[i] = 1;
						flag = 0;
					}
					else if (M_A[i] == 1 && M_B2[i] == 1) {
						M[i] = 1;
						flag = 1;
					}
					else { // 둘 중 하나만 1인 경우
						M[i] = 0;
						flag = 1;
					}
				}
			}
		}else if ((arr_A[0] == 1 && arr_B[0] == 1)) { // A와 B 둘다 음수인 경우 더하기
			for (int i = 12; i >= 0; i--) {
				if (flag == 0) {// 직전 연산에서 넘어온 1이 없다면
					if (M_A[i] == 0 && M_B2[i] == 0)
						M[i] = 0;
					else if (M_A[i] == 1 && M_B2[i] == 1) {
						M[i] = 0;
						flag = 1;
					}
					else { // 둘 중 하나만 1인경우
						M[i] = 1;
					}
				}
				else {
					if (M_A[i] == 0 && M_B2[i] == 0) {
						M[i] = 1;
						flag = 0;
					}
					else if (M_A[i] == 1 && M_B2[i] == 1) {
						M[i] = 1;
						flag = 1;
					}
					else { // 둘 중 하나만 1인 경우
						M[i] = 0;
						flag = 1;
					}
				}
			}
		}
		else if (arr_A[0] == 1 || arr_B[0] == 1) { // 둘 중 하나가 음수인 경우
															//  sign만 빠꿔주면 된다.
			if (arr_A[0] == 1)	// 더 큰 놈이 음수이면 S를 음수로...
				SFParr[0] = 1;
			for (int i = 12; i >= 0; i--) {
				if (flag == 0) {// 직전 연산에서 넘어온 1이 없다면
					if (M_A[i] == 0 && M_B2[i] == 0)
						M[i] = 0;
					else if (M_A[i] == 1 && M_B2[i] == 1) {
						M[i] = 0;
						flag = 0;
					}
					else if (M_A[i] == 0 && M_B2[i] == 1) { // 
						M[i] = 1;
						flag = 1;
					}
					else {
						M[i] = 1;
					}
				}
				else {
					if (M_A[i] == 0 && M_B2[i] == 0) {
						M[i] = 1;
						flag = 1;
					}
					else if (M_A[i] == 1 && M_B2[i] == 0) {
						M[i] = 0;
						flag = 0;
					}
					else if (M_A[i] == 0 && M_B2[i] == 1) { // 둘 중 하나만 1인 경우
						M[i] = 0;
						flag = 1;
					}
					else { // 1 _ 1
						M[i] = 1;
						flag = 1;
					}
				}
			}
		}
		
	}
	else if (E_A < E_B) {
		// E1 - E2 만큼 M2를 뒤로 민다.  그 결과를 M에 저장
		gap = E_B - E_A;
		E = E_B;
		for (int i = 1; i<12; i++) {
			if (i + gap > 12)
				break;
			M_B2[i + gap] = M_A[i];
		}
		// M_B와 M_B2를 더하여 M에 저장한다.
		// M_A와 M_B2를 더하거나 빼서 M에 저장한다.
		if ((arr_A[0] == 0 && arr_B[0] == 0)) { // A와 B 둘다 양수인 경우 더하기
			for (int i = 12; i >= 0; i--) {
				if (flag == 0) {// 직전 연산에서 넘어온 1이 없다면
					if (M_B[i] == 0 && M_B2[i] == 0)
						M[i] = 0;
					else if (M_B[i] == 1 && M_B2[i] == 1) {
						M[i] = 0;
						flag = 1;
					}
					else { // 둘 중 하나만 1인경우
						M[i] = 1;
					}
				}
				else {
					if (M_B[i] == 0 && M_B2[i] == 0) {
						M[i] = 1;
						flag = 0;
					}
					else if (M_B[i] == 1 && M_B2[i] == 1) {
						M[i] = 1;
						flag = 1;
					}
					else { // 둘 중 하나만 1인 경우
						M[i] = 0;
						flag = 1;
					}
				}
			}
		}
		else if ((arr_A[0] == 1 && arr_B[0] == 1)) { // A와 B 둘다 음수인 경우 더하기
			for (int i = 12; i >= 0; i--) {
				if (flag == 0) {// 직전 연산에서 넘어온 1이 없다면
					if (M_B[i] == 0 && M_B2[i] == 0)
						M[i] = 0;
					else if (M_B[i] == 1 && M_B2[i] == 1) {
						M[i] = 0;
						flag = 1;
					}
					else { // 둘 중 하나만 1인경우
						M[i] = 1;
					}
				}
				else {
					if (M_B[i] == 0 && M_B2[i] == 0) {
						M[i] = 1;
						flag = 0;
					}
					else if (M_B[i] == 1 && M_B2[i] == 1) {
						M[i] = 1;
						flag = 1;
					}
					else { // 둘 중 하나만 1인 경우
						M[i] = 0;
						flag = 1;
					}
				}
			}
		}
		else if (arr_A[0] == 1 || arr_B[0] == 1) { // 둘 중 하나가 음수인 경우 빼기
												   //  sign만 빠꿔주면 된다.
			if (arr_B[0] == 1)	// 더 큰 놈이 음수이면 S를 음수로...
				SFParr[0] = 1;
			for (int i = 12; i >= 0; i--) {
				if (flag == 0) {// 직전 연산에서 넘어온 1이 없다면
					if (M_B2[i] == 0&& M_B[i] == 0)
						M[i] = 0;
					else if (M_B2[i] == 1&& M_B[i] == 1 ) {
						M[i] = 0;
						flag = 0;
					}
					else if (M_B2[i] == 0&& M_B[i] ==  1) { // 둘 중 하나만 1인경우
						M[i] = 1;
						flag = 1;
					}
					else {
						M[i] = 1;
					}
				}
				else {
					if (M_B2[i] == 0&& M_B[i] == 0) {
						M[i] = 1;
						flag = 1;
					}
					else if (M_B2[i] == 1&& M_B[i] == 0) {
						M[i] = 0;
						flag = 0;
					}
					else if (M_B2[i] == 0&& M_B[i] == 1) {
						M[i] = 0;
						flag = 1;
					}
					else { // 1 _ 1
						M[i] = 1;
						flag = 1;
					}
				}
			}
		}



	}
	else { // E_A == E_B
		   // M_A와 M_B를 더하여 M에 저장한다.
		E = E_A;
		for (int i = 12; i >= 0; i--) {
			if (flag == 0) {// 직전 연산에서 넘어온 1이 없다면
				if (M_B[i] == 0 && M_A[i] == 0)
					M[i] = 0;
				else if (M_B[i] == 1 && M_A[i] == 1) {
					M[i] = 0;
					flag = 1;
				}
				else { // 둘 중 하나만 1인경우
					M[i] = 1;
				}
			}
			else {
				if (M_B[i] == 0 && M_A[i] == 0) {
					M[i] = 1;
					flag = 0;
				}
				else if (M_B[i] == 1 && M_A[i] == 1) {
					M[i] = 1;
					flag = 1;
				}
				else { // 둘 중 하나만 1인 경우
					M[i] = 0;
					flag = 1;
				}
			}
		}
	}
	// 도출된 M 과 E에 대하여 연산하여 sfp를 만들어 낸다.

	// overflow 처리 (연산결과가 exp > 62 인경우 )
	int exp = E + Bias;
	if (exp == 62 && M[0] == 1) {
		for (int i = 0; i < 16; i++) {
			if (PlusInfinityArr[i] == 0) {
				result = result << 1;
			}
			else { // 1이면
				result = result << 1;
				result++;
			}
		}
		return result;
	}
	// 연산 결과가 carry되었을 경우
		// exp를 1증가시킨다.
		// M배열을 뒤로 한칸씩 민다.
	int Mtemp[12] = { 0, };
	for (int i = 0; i < 12; i++) {
		Mtemp[i] = M[i];
	}
	if (M[0] == 1) {
		exp++;
		for (int i = 0; i < 11; i++) {
			M[i + 1] = Mtemp[i];
		}
	}
	/*
	// 연산 결과가 decarry되었을 경우
	// M배열 처음의 1을 발견한다.
	if (M[1] == 0 && M[0] == 0) {
		int idx = 0;
		for (idx = 0; idx < 12; idx++) {
			if (M[idx] == 1)
				break;
		}
		exp -= (idx - 1);
		for (int i = 0; i < 12; i++) {
			Mtemp[i] = M[i];
		}


		for (int i = idx; i < 11; i++) {
			M[i + 1] = Mtemp[i];
		}
	

	}
	*/



	// Round-to-even 적용
	int M_C[12] = { 0,0,0,0,0,0,0,0,0,0,0,1 };
	flag = 0;
	if (M[10] == 1 && M[11] == 1) { // 1을 더한다.
		for (int i = 11; i >= 0; i--) {
			if (flag == 0) {// 직전 연산에서 넘어온 1이 없다면
				if (M[i] == 0 && M_C[i] == 0)
					M[i] = 0;
				else if (M[i] == 1 && M_C[i] == 1) {
					M[i] = 0;
					flag = 1;
				}
				else { // 둘 중 하나만 1인경우
					M[i] = 1;
				}
			}
			else {
				if (M[i] == 0 && M_C[i] == 0) {
					M[i] = 1;
					flag = 0;
				}
				else if (M[i] == 1 && M_C[i] == 1) {
					M[i] = 1;
					flag = 1;
				}
				else { // 둘 중 하나만 1인 경우
					M[i] = 0;
					flag = 1;
				}
			}
		}
	}
	// sign bit SFParr에 담기
	if (arr_A[0] == 1 && arr_B[0] == 1)
		SFParr[0] = 1;
	
	// exp 2진수 변환 후 SFParr에 담기
	int TempBinary2[6] = { 0, }; // 0<= exp <= 62  
	int idx2 = 0;
	while (1) {
		TempBinary2[idx2++] = exp % 2;
		exp = exp / 2;
		if (exp == 0)
			break;
	}
	// exp를 담는다.
	for (int i = 5; i >= 0; i--) {
		SFParr[5 - i + 1] = TempBinary2[i];
	}
	// frac을 담는다.
	for (int i = 1; i <10; i++) {
		SFParr[i + 6] = M[i + 1];
	}

	// SFP배열의 비트를 통해 result의 비트를 조작하여 반환
	for (int i = 0; i < 16; i++) {
		if (SFParr[i] == 0) {
			result = result << 1;
		}
		else { // 1이면
			result = result << 1;
			result++;
		}
	}
	return result;

}


char* sfp2bits(sfp result) {
	// 쉬프트 연산자를 이용하여 하나씩 저장한다.
	char* String = (char*)malloc(sizeof(char) * 17);
	// num 의 비트 단위 확인
	int num = 0;
	int j = 0;
	// 비트단위로 골라내어 arr에 저장하기
	for (int i = 0; i < 16; i++) {
		num = (result & 1 << i) >> i;
		if (num == 0)
			String[15 - j++] = '0';
		else
			String[15 - j++] = '1';
	}
	String[16] = '\0';

	return String;
}
