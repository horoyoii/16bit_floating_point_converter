#include "sfp.h"
#include<stdlib.h>
#include<math.h>

sfp int2sfp(int input) {
	// 1. �Ű����� int�� input�� 2������ ��ȯ�Ѵ�.
	// 2. (-1)^s * M * 2^E ���·� ��ȯ�Ѵ�.
	// 3. S �� ���Ͽ� 
	// 4. E �� ���Ͽ�
	// 4-1. (��ȯ�� E)exp �� 62���� Ŀ���� overflow ó�� ,S�� 0 �̸� +���Ѵ�, S�� 1�̸� -���Ѵ�
	// 4-2. exp�� 2������ ��ȯ�Ͽ� sfp�� ��´�.
	// 5. M �� ���Ͽ�
	// 5-1. (��ȯ�� M) frac�� 9bit �ʰ��� ���� ���
	// 5-2. Round-to-zero mode�� ���� 10bit���ʹ� �׳� drop�Ѵ�.

	sfp result = 0; // ��ȯ ��
	int target = input;
	int SFParr[16] = { 0, };

	if (input == 0)
		return result;

	// 1. 2������ ��ȯ
	int TempBinary[31] = { 0, };
	int Binary[31] = { 0, }; // result�� 2������ ��ȯ�Ͽ� ��´�.
	int idx = 0;
	while (1) {
		TempBinary[idx++] = target % 2;
		target = target / 2;
		if (target == 0)
			break;
	}
	int j = 0;	// j = ��Ʈ ������ ����
	for (int i = idx - 1; i >= 0; i--, j++) {  // �Ųٷ� ����� 2������ �ùٸ��� Binary�� ��´�.
		Binary[j] = TempBinary[i];
	}

	// 3. S�� ���Ͽ�
	if (input > 0)
		SFParr[0] = 0;
	else
		SFParr[0] = 1;

	// 4. E�� ���Ͽ�
	int E;
	int Bias = 31;
	int exp;
	E = j - 1;	// j�� ��Ʈ������ ��� �ֱ⿡ E�� ���� �� �ִ�.
	exp = E + Bias;

	// 4-1.overflowó��
	int PlusInfinityArr[16] = { 0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0 };
	int MinusInfinityArr[16] = { 1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0 };
	if (exp > 62) {
		if (SFParr[0] == 0) { // ���� ���Ѵ�
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
		else { // ���� ���Ѵ�
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
		
	// 4.2 exp 2���� ��ȯ �� SFParr�� ���
	int TempBinary2[6] = { 0, }; // 0<= exp <= 62  
	int idx2 = 0;
	while (1) {
		TempBinary2[idx2++] = exp % 2;
		exp = exp / 2;
		if (exp == 0)
			break;
	}
	for (int i = 5; i >= 0; i--) { // exp�� ��´�.
		SFParr[5 - i + 1] = TempBinary2[i];
	}

	// 5. M�� ���Ͽ�
	// 5.1 Binary�迭�� 1����Һ��� 9��°��ұ��� ��´�.
	// 5.2 Round-to-zero �̱⿡ 10��°�� �ڵ� drop
	for (int i = 1; i <10; i++) {
		SFParr[i + 6] = Binary[i];
	}

	// 6. SFP�迭�� ��Ʈ�� ���� result�� ��Ʈ�� �����Ͽ� ��ȯ
	for (int i = 0; i < 16; i++) {
		if (SFParr[i] == 0) {
			result = result << 1;
		}
		else { // 1�̸�
			result = result << 1;
			result++;
		}
	}
	return result;
}


int sfp2int(sfp input) {
	// 1. sfp�� input�� arr�迭�� ��Ʈ������ ����ش�.
	// 2. ���� ó��
	// 2-1. ���� ���Ѵ��� ��� // TMAX���� ��ȯ
	// 2-2. ���� ���Ѵ��� ��� // TMIN���� ��ȯ
	// 2-3. Nan�� ���		// TMIN���� ��ȯ
	// 3. Sign�� ����
	// 4. Exp�� ����
	// 5. Frac�� ����
	// 6. 10���� ��ȯ
	int result = 0;

	// 1. input�� ��Ʈ������ arr�� ����
	int arr[16] = { 0, };
	int num = 0;
	int j = 0;
	for (int i = 0; i < 16; i++) {
		num = (input & 1 << i) >> i;
		arr[15 - j++] = num;
	}

	//2. ���� ó��
	// 2-1. ���� ���Ѵ��� ��� // TMAX���� ��ȯ
	// 2-2. ���� ���Ѵ��� ��� // TMIN���� ��ȯ
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

	// 2-3. Nan�� ���		// TMIN���� ��ȯ
	int Nan = 0;
	for (int i = 1; i < 7; i++) {
		if (arr[i] == 1)
			Nan++;
	}
	for (int i = 7; i < 16; i++) {
		if (arr[i] == 1 && Nan == 6)	// exp�� ���� 1�̰� frac�� 000...0 �� �ƴ϶��. 
			return -2147483648;
	}

	// 3. sign�� ȭ�� �� sign������ ������ ����� �����ش�.
	int sign = 0;
	if (arr[0] == 1)
		sign = -1;
	else
		sign = 1;

	// 4. Exp��
	int exp = 0;
	int E;
	int Bias = 31;
	for (int j = 6; j >= 1; j--) {
		if (arr[j] == 1) {
			exp += 1 << (6 - j);
		}
	}
	E = exp - Bias;

	// 5. Frac��
	int Frac[10] = { 0, };
	for (int i = 0; i < 9; i++) {
		Frac[i + 1] = arr[i + 7];
	}
	Frac[0] = 1;

	//6. 10���� ��ȯ
	for (int j = E; j >= 0; j--) {
		if (Frac[j] == 1) {
			result += 1 << (E - j);
		}
	}

	return result * sign;

}

sfp float2sfp(float input) {
	// 1. sign�� ����
	// 2. float�� input�� ���Ͽ� 2������ ��ȯ�Ѵ�.

	sfp result = 0;

	float target = input;
	int SFParr[16] = { 0, };

	//1. sign�� ����
	if (target > 0)
		SFParr[0] = 0;
	else
		SFParr[0] = 1;

	// 2. float�� input�� ���Ͽ� 2������ ��ȯ�Ѵ�.
	// 2-1. �Ҽ��� ������ ������ ������ 2������ ��ȯ�Ѵ�.
	int Front = 0; // �Ҽ��� ����
	float Back = 0; // �Ҽ��� ����
	if (target > 0) {
		Front = input;
		Back = input - Front;
	}
	else {
		Front = input * -1;
		Back = (input + Front)*-1;
	}

	int Binary[200] = { 0, }; // input�� 2������ ��ȯ�Ͽ� ��´�.
	int TempBinary[200] = { 0, };

	// 2-2. Front �� 2���� ��ȯ
	int idx = 0;
	if (Front != 0) {	// 0.xx�� �ƴ� ���
		while (1) {

			TempBinary[idx++] = Front % 2;
			Front = Front / 2;

			if (Front == 0)
				break;
		}
	}
	int j = 0;	// j = ��Ʈ ������ ����, �Ҽ��� ���ۺκ��� �ǹ��ϴ� �ε����̴�.
	for (int i = idx - 1; i >= 0; i--, j++) { // �Ųٷ� ����� 2������ �ùٸ��� Binary�� ��´�.
		Binary[j] = TempBinary[i];
	}

	// 2-3. Back �� 2���� ��ȯ
	//  Binary�迭�� j��° �ε������� ��´�.
	// �Ҽ��ΰ� 0���� �������� �ʴ´ٸ� ��Ʈ�� ����ؼ� ���´�. So Binary�迭�� 200-j ����ŭ�� �����Ѵ�.
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

	// 3. exp �� ���� 
	int E = 0;
	int Bias = 31;
	int exp;
	int TempBinary2[6] = { 0, }; // exp�� 2������ ��ȯ�Ѵ�.
	int idx2 = 0;
	int FracIdx = 7;
	// -1. case �з�
	// 1. 0.xxx�� �� ���
	if (j == 0) { // 0.xxx �� �� ���  �Ҽ������� ó�� �����ϴ� 1�� ã�´�.
		while (1) {
			if (Binary[E++] == 1) {
				exp = E * -1 + Bias;
				break;
			}
		}
		// ����ó��
		// exp < 0 �� ��� ���� �ſ� �۾� ǥ���� �� ����.  0�� �����Ѵ�.
		if (exp < 0)
			return 0;
		while (1) {
			TempBinary2[idx2++] = exp % 2;
			exp = exp / 2;
			if (exp == 0)
				break;
		}
		// exp�� ��´�.
		for (int i = 5; i >= 0; i--) {
			SFParr[5 - i + 1] = TempBinary2[i];
		}
		// frac�� ��´�.
		for (int i = 0; i < 9; i++) {	// E�� Binary���� 1�� ó�� �����ϴ� �ε���+1�̴�. 
			SFParr[FracIdx++] = Binary[E++]; // Round-to-zero�̱⿡ Frac�� 9bit�� ��� �������� drop�Ѵ�.
		}
	}
	else { // xxx.xxx �� �� ���
		E = j - 1;
		exp = E + Bias;

		// ����ó�� : overflowó��
		int PlusInfinityArr[16] = { 0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0 };
		int MinusInfinityArr[16] = { 1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0 };
		if (exp > 62) {
			if (SFParr[0] == 0) { // ���� ���Ѵ�
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
			else { // ���� ���Ѵ�
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
		// exp�� ��´�.
		for (int i = idx2 - 1, k = 1; i >= 0; i--, k++) {
			SFParr[k] = TempBinary2[i];
		}
		// frac�� ��´�.
		for (int i = 0; i < 9; i++) {
			SFParr[FracIdx++] = Binary[i + 1];
		}
	}

	// 5. result�� ����ش�.
	for (int i = 0; i < 16; i++) {
		if (SFParr[i] == 0) {
			result = result << 1;
		}
		else { // 1�̸�
			result = result << 1;
			result++;
		}

	}

	return result;
}

float sfp2float(sfp input) {
	// ��Ʈ ������ �迭�� ��´�
	float result = 0;

	// arr�迭�� input�� ��Ʈ�� ��´�.
	int arr[16] = { 0, };
	int num = 0;
	int j = 0;
	for (int i = 0; i < 16; i++) {
		num = (input & 1 << i) >> i;
		arr[15 - j++] = num;
	}

	// ���� ó�� : ���Ѵ�
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

	//����ó�� :  Nan 
	int Nan = 0;
	for (int i = 1; i < 7; i++) {
		if (arr[i] == 1)
			Nan++;
	}
	for (int i = 7; i < 16; i++) {
		if (arr[i] == 1 && Nan == 6)
			return -340282346638528859811704183484516925440.00;
	}

	// 3. sign�� ����
	// sign�� ȭ�� �� sign������ ������ ����� �����ش�.
	int sign = 0;
	if (arr[0] == 1)
		sign = -1;
	else
		sign = 1;

	// 4. exp�� ����
	int exp = 0;
	int E;
	int Bias = 31;
	for (int j = 6; j >= 1; j--) {
		if (arr[j] == 1) {
			exp += 1 << (6 - j);
		}
	}
	E = exp - Bias;

	// 5. Frac�� ����
	int Frac[10] = { 0, };
	Frac[0] = 1;

	for (int i = 0; i < 9; i++) {
		Frac[i + 1] = arr[i + 7];
	}

	// 10���� ��ȯ
	// �����κ�
	for (int j = 0; j < 10; j++) {
		if (Frac[j] == 1) {
			result += pow(2, E - j);
		}
	}


	return result * sign;
}

sfp sfp_add(sfp a, sfp b) {
	// Round-to-even�� �����Ų��.
	// 1. a�� b�� (-1)^s * M * 2^E������ ��ȯ�Ѵ�.
	// 2. E1�� E2�� ���Ͽ� ū ������ �����Ѵ�.
	// 3. M1�� M2�� �����ش�.
	// 3-1. ������ M�� ���Ͽ� 9���� 10�� ��Ʈ�� ���� �ݿø��� �����Ѵ�.
	// 3-1. ���� ����� exp >62�� ��� overflow
	//4. ���� ����� (-1)^s* M * 2^E�� sfp�� �������ش�.

	sfp result;
	int SFParr[16] = { 0, };
	// 1. a�� ���Ͽ�
	int arr_A[16] = { 0, };
	int num_A = 0;
	int j_A = 0;
	for (int i = 0; i < 16; i++) {
		num_A = (a & 1 << i) >> i;
		arr_A[15 - j_A++] = num_A;
	}
	// ������
	int exp_A = 0;
	int E_A;
	int Bias = 31;
	for (int j = 6; j >= 1; j--) {
		if (arr_A[j] == 1) {
			exp_A += 1 << (6 - j);
		}
	}
	E_A = exp_A - Bias;

	// 1. b�� ���Ͽ�
	int arr_B[40] = { 0, };
	int num_B = 0;
	int j_B = 0;
	for (int i = 0; i < 16; i++) {
		num_B = (b & 1 << i) >> i;
		arr_B[15 - j_B++] = num_B;
	}

	// ������
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
		// +���Ѵ� +���Ѵ��� ���
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
		// +���Ѵ� -���Ѵ��� ���
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
		// -���Ѵ� +���Ѵ��� ���
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
		// -���Ѵ� -���Ѵ��� ���
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
		// �ϳ��� +���Ѵ��� ���
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
		// �ϳ��� -���Ѵ��� ���
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
		//����ó�� :  Nan 
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


	// 2. E1 ��  E2�� ���Ͽ� ū ���� �����.
	int M_A[13] = { 0,1,0,0,0,0,0,0,0,0,0,0,0 };
	for (int i = 0; i < 9; i++) {
		M_A[i + 2] = arr_A[i + 7];
	}
	int M_B[13] = { 0,1,0,0,0,0,0,0,0,0,0,0,0 };
	for (int i = 0; i < 9; i++) {
		M_B[i + 2] = arr_B[i + 7];
	}
	int M_B2[13] = { 0, }; // E�� ���� M�� ������ M
	int M[13] = { 0, };
	int E;
	int gap;
	int flag = 0;
	if (E_A > E_B) {
		// E1 - E2 ��ŭ M2�� �ڷ� �δ�.  �� ����� M�� ����
		gap = E_A - E_B;
		E = E_A;
		for (int i = 1; i<12; i++) {
			if (i + gap > 12)
				break;
			M_B2[i + gap] = M_B[i];
		}
		// M_A�� M_B2�� ���ϰų� ���� M�� �����Ѵ�.
		if ((arr_A[0] == 0 && arr_B[0] == 0)) { // A�� B �Ѵ� ����� ��� ���ϱ�
			for (int i = 12; i >= 0; i--) {
				if (flag == 0) {// ���� ���꿡�� �Ѿ�� 1�� ���ٸ�
					if (M_A[i] == 0 && M_B2[i] == 0)
						M[i] = 0;
					else if (M_A[i] == 1 && M_B2[i] == 1) {
						M[i] = 0;
						flag = 1;
					}
					else { // �� �� �ϳ��� 1�ΰ��
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
					else { // �� �� �ϳ��� 1�� ���
						M[i] = 0;
						flag = 1;
					}
				}
			}
		}else if ((arr_A[0] == 1 && arr_B[0] == 1)) { // A�� B �Ѵ� ������ ��� ���ϱ�
			for (int i = 12; i >= 0; i--) {
				if (flag == 0) {// ���� ���꿡�� �Ѿ�� 1�� ���ٸ�
					if (M_A[i] == 0 && M_B2[i] == 0)
						M[i] = 0;
					else if (M_A[i] == 1 && M_B2[i] == 1) {
						M[i] = 0;
						flag = 1;
					}
					else { // �� �� �ϳ��� 1�ΰ��
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
					else { // �� �� �ϳ��� 1�� ���
						M[i] = 0;
						flag = 1;
					}
				}
			}
		}
		else if (arr_A[0] == 1 || arr_B[0] == 1) { // �� �� �ϳ��� ������ ���
															//  sign�� �����ָ� �ȴ�.
			if (arr_A[0] == 1)	// �� ū ���� �����̸� S�� ������...
				SFParr[0] = 1;
			for (int i = 12; i >= 0; i--) {
				if (flag == 0) {// ���� ���꿡�� �Ѿ�� 1�� ���ٸ�
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
					else if (M_A[i] == 0 && M_B2[i] == 1) { // �� �� �ϳ��� 1�� ���
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
		// E1 - E2 ��ŭ M2�� �ڷ� �δ�.  �� ����� M�� ����
		gap = E_B - E_A;
		E = E_B;
		for (int i = 1; i<12; i++) {
			if (i + gap > 12)
				break;
			M_B2[i + gap] = M_A[i];
		}
		// M_B�� M_B2�� ���Ͽ� M�� �����Ѵ�.
		// M_A�� M_B2�� ���ϰų� ���� M�� �����Ѵ�.
		if ((arr_A[0] == 0 && arr_B[0] == 0)) { // A�� B �Ѵ� ����� ��� ���ϱ�
			for (int i = 12; i >= 0; i--) {
				if (flag == 0) {// ���� ���꿡�� �Ѿ�� 1�� ���ٸ�
					if (M_B[i] == 0 && M_B2[i] == 0)
						M[i] = 0;
					else if (M_B[i] == 1 && M_B2[i] == 1) {
						M[i] = 0;
						flag = 1;
					}
					else { // �� �� �ϳ��� 1�ΰ��
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
					else { // �� �� �ϳ��� 1�� ���
						M[i] = 0;
						flag = 1;
					}
				}
			}
		}
		else if ((arr_A[0] == 1 && arr_B[0] == 1)) { // A�� B �Ѵ� ������ ��� ���ϱ�
			for (int i = 12; i >= 0; i--) {
				if (flag == 0) {// ���� ���꿡�� �Ѿ�� 1�� ���ٸ�
					if (M_B[i] == 0 && M_B2[i] == 0)
						M[i] = 0;
					else if (M_B[i] == 1 && M_B2[i] == 1) {
						M[i] = 0;
						flag = 1;
					}
					else { // �� �� �ϳ��� 1�ΰ��
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
					else { // �� �� �ϳ��� 1�� ���
						M[i] = 0;
						flag = 1;
					}
				}
			}
		}
		else if (arr_A[0] == 1 || arr_B[0] == 1) { // �� �� �ϳ��� ������ ��� ����
												   //  sign�� �����ָ� �ȴ�.
			if (arr_B[0] == 1)	// �� ū ���� �����̸� S�� ������...
				SFParr[0] = 1;
			for (int i = 12; i >= 0; i--) {
				if (flag == 0) {// ���� ���꿡�� �Ѿ�� 1�� ���ٸ�
					if (M_B2[i] == 0&& M_B[i] == 0)
						M[i] = 0;
					else if (M_B2[i] == 1&& M_B[i] == 1 ) {
						M[i] = 0;
						flag = 0;
					}
					else if (M_B2[i] == 0&& M_B[i] ==  1) { // �� �� �ϳ��� 1�ΰ��
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
		   // M_A�� M_B�� ���Ͽ� M�� �����Ѵ�.
		E = E_A;
		for (int i = 12; i >= 0; i--) {
			if (flag == 0) {// ���� ���꿡�� �Ѿ�� 1�� ���ٸ�
				if (M_B[i] == 0 && M_A[i] == 0)
					M[i] = 0;
				else if (M_B[i] == 1 && M_A[i] == 1) {
					M[i] = 0;
					flag = 1;
				}
				else { // �� �� �ϳ��� 1�ΰ��
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
				else { // �� �� �ϳ��� 1�� ���
					M[i] = 0;
					flag = 1;
				}
			}
		}
	}
	// ����� M �� E�� ���Ͽ� �����Ͽ� sfp�� ����� ����.

	// overflow ó�� (�������� exp > 62 �ΰ�� )
	int exp = E + Bias;
	if (exp == 62 && M[0] == 1) {
		for (int i = 0; i < 16; i++) {
			if (PlusInfinityArr[i] == 0) {
				result = result << 1;
			}
			else { // 1�̸�
				result = result << 1;
				result++;
			}
		}
		return result;
	}
	// ���� ����� carry�Ǿ��� ���
		// exp�� 1������Ų��.
		// M�迭�� �ڷ� ��ĭ�� �δ�.
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
	// ���� ����� decarry�Ǿ��� ���
	// M�迭 ó���� 1�� �߰��Ѵ�.
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



	// Round-to-even ����
	int M_C[12] = { 0,0,0,0,0,0,0,0,0,0,0,1 };
	flag = 0;
	if (M[10] == 1 && M[11] == 1) { // 1�� ���Ѵ�.
		for (int i = 11; i >= 0; i--) {
			if (flag == 0) {// ���� ���꿡�� �Ѿ�� 1�� ���ٸ�
				if (M[i] == 0 && M_C[i] == 0)
					M[i] = 0;
				else if (M[i] == 1 && M_C[i] == 1) {
					M[i] = 0;
					flag = 1;
				}
				else { // �� �� �ϳ��� 1�ΰ��
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
				else { // �� �� �ϳ��� 1�� ���
					M[i] = 0;
					flag = 1;
				}
			}
		}
	}
	// sign bit SFParr�� ���
	if (arr_A[0] == 1 && arr_B[0] == 1)
		SFParr[0] = 1;
	
	// exp 2���� ��ȯ �� SFParr�� ���
	int TempBinary2[6] = { 0, }; // 0<= exp <= 62  
	int idx2 = 0;
	while (1) {
		TempBinary2[idx2++] = exp % 2;
		exp = exp / 2;
		if (exp == 0)
			break;
	}
	// exp�� ��´�.
	for (int i = 5; i >= 0; i--) {
		SFParr[5 - i + 1] = TempBinary2[i];
	}
	// frac�� ��´�.
	for (int i = 1; i <10; i++) {
		SFParr[i + 6] = M[i + 1];
	}

	// SFP�迭�� ��Ʈ�� ���� result�� ��Ʈ�� �����Ͽ� ��ȯ
	for (int i = 0; i < 16; i++) {
		if (SFParr[i] == 0) {
			result = result << 1;
		}
		else { // 1�̸�
			result = result << 1;
			result++;
		}
	}
	return result;

}


char* sfp2bits(sfp result) {
	// ����Ʈ �����ڸ� �̿��Ͽ� �ϳ��� �����Ѵ�.
	char* String = (char*)malloc(sizeof(char) * 17);
	// num �� ��Ʈ ���� Ȯ��
	int num = 0;
	int j = 0;
	// ��Ʈ������ ��󳻾� arr�� �����ϱ�
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
