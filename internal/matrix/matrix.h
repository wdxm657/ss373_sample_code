/*
\ \     __  ___     __      _       __ __     __
 \ \   /  |/  /__ _/ /_____(_)_ __ / // /_ __/ /
 / /  / /|_/ / _ `/ __/ __/ /\ \ // _  / // / _ \
/ /  /_/  /_/\_,_/\__/_/ /_//_\_\/_//_/\_,_/_.__/
* [INFORMATION]
    MATRIX_HUB
    AUTHOR: Xiping.Yu
    E-MAIL:Amoiensis@outlook.com
    GITHUB: https://github.com/Amoiensis/Matrix_hub
    DATE: 2020.02.12-2023.08.21
    VERSION: 1.5.2
    CASE: Matrix Operation (C)
    DETAILS: The code_file for Matrix_Hub.
* [LICENSE]
    Copyright (c) 2020-2022 Xiping.Yu
    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at
        http://www.apache.org/licenses/LICENSE-2.0
    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#ifndef _ST_MATRIX_H_
#define _ST_MATRIX_H_

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>
#include <string.h>


/* Num Type of Matrix - 矩阵数值类型*/
#define MATRIX_TYPE double
#define TRANS_TYPE  double
/* Displayed Precision - 输出显示精度*/
#define PRECISION "%-16.6lf\t"
/* The Level of Details of Output - 显示详细等级*/
#define _DETAILED_ 0
/* 档位: 0/1/2/3 四等级: 0->3 逐渐详细 (default-2)
 * level - 显示详情的函数
 * 0 - M_print （除设定的输出外，不额外显示其他计算细节信息）
 * 1 - M_Uptri_/ M_Lowtri_/ M_Diatri_
 * 2 - M_full/ M_Inverse/ M_eigen_val/ M_rank / M_Uptri_/ M_Lowtri_/ M_Diatri_/ M_print
 * 3 - M_free/ M_mul/ M_full/ M_Inverse/ M_rank/ M_mul / M_Uptri_/ M_Lowtri_/ M_Diatri_/ M_print
 * */
/* 进度条显示 */
#define _progress_bar_ 1


/* Control Constant - 设置的常数*/
#define _FULLY_BIG_    0x3f3f3f3f
#define _INFINITE_    1000

#define _MAX_HELP_LENGTH_ 30

#define _ROW_    1
#define _COLUMN_ 0

#define _END_    -1
#define _HEAD_    1

#define _ONE_ 1
#define _ZERO_ 0

#define _AND_ 1
#define _OR_ 0
#define _NOT_ -1

#define _MUL_ 1
#define _DIV_ -1

#define _INV_L_ 0
#define _INV_R_ 1
#define _SVD_ 3

#define _ORD4INI_ 1
#define _ORD4VAL_ -1

#define INF INT_MAX
#define FRO INT_MIN

#define _MAX_LOOP_NUM_ (int)1e+5

#define _APPROXIMATELY_ZERO_ (1e-5f)


/*ERROR TABLE - 错误提示*/
#define M_mul_001        "@ERROR: Matrix_Dimensions Wrong!\n\tDetails:(M_mul_001)_mat_left->column != _mat_right->row\n"
#define M_Dia_Inv_002    "@ERROR: Matrix_Dimensions Wrong!\n\tDetails:(M_Dia_Inv_002)_mat_left->column != _mat_right->row\n"
#define M_add_sub_003    "@ERROR: Matrix_Dimensions Wrong!\n\tDetails:(M_add_sub_003)_mat_subed != _mat_minus\n"
#define M_swap_004        "@ERROR: Matrix_Swap_Line Over!\n\tDetails:(M_swap_004)_Swap_line over the limited\n"
#define M_Cut_005        "@ERROR: Matrix_Cut Over!\n\tDetails:(M_Cut_005)_Cut_tail over_the limited\n"
#define M_Cut_006        "@ERROR: Matrix_Cut Wrong!\n\tDetails:(M_Cut_006)_Head_>_Tail\n"
#define M_Cut_007        "@ERROR: Matrix_Cut Wrong!\n\tDetails:(M_Cut_007)_Range_can't_be_negative!'\n"
#define M_tr_008        "@ERROR: Matrix_trace Wrong!\n\tDetails:(M_tr_008)_ROW_!=_COLUMN.'\n"
#define M_det_009        "@ERROR: Matrix_Determinant_ Wrong!\n\tDetails:(M_det_009)_ROW_!=_COLUMN.'\n"
#define M_Mfull_010    "@ERROR: M_matFull Wrong!\n\tDetails:(M_Mfull_010)_ROW_OVER!.'\n"
#define M_Mfull_011    "@ERROR: M_matFull Wrong!\n\tDetails:(M_Mfull_011)_COLUMN_OVER!.'\n"
#define M_logic_012        "@ERROR: Matrix_Dimensions Wrong!\n\tDetails:(M_logic_012)_mat_left->row != _mat_right->row\n"
#define M_logic_013    "@ERROR: Matrix_Dimensions Wrong!\n\tDetails:(M_logic_013)_mat_left->column != _mat_right->column\n"
#define M_logic_014    "@ERROR: Operation Wrong(Dont Exist)!\n\tDetails:(M_logic_014)Operation_Wrong !\n"
#define M_pmuldiv_015    "@ERROR: Matrix_Dimensions Wrong!\n\tDetails:(M_pmuldiv_015)_mat_left->row != _mat_right->row\n"
#define M_pmuldiv_016    "@ERROR: Matrix_Dimensions Wrong!\n\tDetails:(M_pmuldiv_016)_mat_left->column != _mat_right->column\n"
#define M_pmuldiv_017    "@ERROR: Operation Wrong(Dont Exist)!\n\tDetails:(M_pmuldiv_017)Operation_Wrong !\n"
#define M_setval_018    "@ERROR: Mat_order lager than Mat_ini !\n\tDetails:(M_setval_018)Mat_order_Size_Wrong !\n"
#define M_setval_019    "@ERROR: Mat_order lager than Mat_val !\n\tDetails:(M_setval_019)Mat_order_Size_Wrong !\n"
#define M_setval_020    "@ERROR: Mat_ini lager than Mat_order !\n\tDetails:(M_setval_020)Mat_ini_Size_Wrong !\n"
#define M_eigen_max_021    "@ERROR: Matrix_Dimensions Wrong!\n\tDetails:(M_eigen_max_021)Mat->column != Mat->row!\n\t\t(For eigen, the Matrix must be a square matrix!)\n"
#define M_norm_022        "@ERROR: M_norm Wrong!\n\t Details:(M_norm_022)The Norm-Setting should be 1/2/INF/p for Vector and 1/2/INF/FRO for Matrix!\n"
#define M_Dia_Inv_023    "@ERROR: Matrix is not invertible!\n\t Details:(M_Dia_Inv_023)Please Check: Inverse element of Dia == 0! \n"
#define M_cond_024    "@ERROR: M_cond Wrong! \n\t Details:(M_cond_024) Matrix should be Square-Matrix! _mat_left->row != _mat_right->row \n"
#define M_cond_025    "@ERROR: M_cond Wrong!\n\t Details:(M_cond_024) M_cond (mat, int Setting <<-- should be 1/2/INF/FRO) \n"
#define M_eigen_026    "@ERROR: Matrix_Dimensions Wrong!\n\tDetails:(M_eigen_026)Mat->column != Mat->row!\n\t\t(For eigen, the Matrix must be a square matrix!)\n"
#define M_householder_027    "@ERROR: Matrix_Dimensions Wrong!\n\tDetails:(M_householder_027)Mat->column != Mat->row!\n\t\t(For Matrix_householder, the Matrix must be a square matrix!)\n"
#define M_pinv_028    "@ERROR: M_pinv Model Wrong!\n\tDetails:(M_pinv_028)\n\t\tModel: _LEFT_, _RIGHT_, _SVD_.\n"

/*WARNING TABLE - 警告提示*/
#define M_norm_warm_01        "@WARNING: ||A||_p = sum((a_ij)^p)^(1/p)\n\tFor matrix's p-normvalue, the result can not be a complex number! (e.g. A+Bi)\n"
#define householder_warm_02   "@WARNING: ||vec||_2 = _APPROXIMATELY_ZERO_\n\tFor householder, the vector better not equal to vec_0.\n"



typedef struct _Matrix {
    /*Store Matrix
	存储矩阵*/
    int row;
    int column;
    MATRIX_TYPE *data;
} Matrix;

typedef struct _Elementary_Transformation {
    /*Store the Operation of Elementary_Transformation
	存储初等变化的运算过程*/
    int minuend_line;
    int subtractor_line;
    TRANS_TYPE scale;
    struct _Elementary_Transformation *forward_E_trans;
    struct _Elementary_Transformation *next_E_trans;
} Etrans_struct;

typedef struct _Upper_triangular_transformation {
    /*Store the result of Upper_triangular_transformation
	存储上三角化的运算结果*/
    Matrix *trans_matrix;
    Matrix *Uptri_matrix;
} Uptri_struct;

typedef struct _Lower_triangular_transformation {
    /*Store the result of Upper_triangular_transformation
	存储下三角化的运算结果*/
    Matrix *trans_matrix;
    Matrix *Lowtri_matrix;
} Lowtri_struct;

typedef struct _Diagonalization_transformation {
    /*Store the result of Upper_triangular_transformation
	存储对角化化的运算结果*/
    Matrix *trans_leftmatrix;
    Matrix *Diatri_matrix;
    Matrix *trans_rightmatrix;
} Dia_struct;

typedef struct _matrix_inverse_struct {
    /*Store the result of matrix_inverse
	存储求逆运算的中间结果，提高算法效率*/
    Matrix *_matrix;
    struct _Elementary_Transformation *_Etrans_head;
} M_inv_struct;

typedef struct _matrix_eigen_struct_single {
    /*Store the result of matrix_eigen
	存储求最大特征值运算的结果*/
    Matrix *eigen_matrix;
    double eigen_value;
} M_eigen_struct;


Matrix *Matrix_gen(int row, int column, MATRIX_TYPE *data);

Matrix *Matrix_copy(Matrix *_mat_sourse);

Matrix *M_mul(Matrix *_mat_left, Matrix *_mat_right);

Matrix *M_add_sub(MATRIX_TYPE scale_mat_subed, Matrix *_mat_subed, MATRIX_TYPE scale_mat_minus, Matrix *_mat_minus);

int M_print(Matrix *_mat);

Matrix *M_I(int order);

int M_E_trans(Matrix *_mat, Etrans_struct *_Etrans_, int line_setting);

Matrix *Etrans_2_Matrix(Etrans_struct *_Etrans_, int order, int line_setting);

Matrix *Etrans_4_Inverse(Matrix *_mat_result, Etrans_struct *_Etrans_, int line_setting);

Uptri_struct *M_Uptri_(Matrix *_mat_source);

M_inv_struct *M_Uptri_4inv(Matrix *_mat_source);

Lowtri_struct *M_Lowtri_(Matrix *_mat_source);

M_inv_struct *M_Lowtri_4inv(Matrix *_mat_source);

Matrix *M_Dia_Inv(Matrix *_mat_source);

Dia_struct *M_Diatri_(Matrix *_mat_source);

Matrix *M_Inverse(Matrix *_mat);

int M_Swap(Matrix *_mat, int _line_1, int _line_2, int line_setting);

Matrix *M_Cut(Matrix *_mat, int row_head, int row_tail, int column_head, int column_tail);

Matrix *M_T(Matrix *_mat_source);

int M_free(Matrix *_mat);

MATRIX_TYPE M_tr(Matrix *_mat);

MATRIX_TYPE M_det(Matrix *_mat);

Matrix *M_full(Matrix *_mat, int row_up, int row_down, int column_left, int column_right, MATRIX_TYPE full_data);

MATRIX_TYPE M_norm(Matrix *_mat, int Setting);

Matrix *M_abs(Matrix *_mat_origin);

Matrix *M_numul(Matrix *_mat, MATRIX_TYPE _num);

Matrix *M_matFull(Matrix *_mat, int row_up, int column_left, Matrix *_mat_full);

Matrix *M_Zeros(int row, int column);

Matrix *M_Ones(int row, int column);

Matrix *M_find(Matrix *_mat, MATRIX_TYPE value);

Matrix *M_sum(Matrix *_mat);

int Min_position(MATRIX_TYPE *data, int size);

Matrix *M_min(Matrix *_mat);

Matrix *M_max(Matrix *_mat);

Matrix *M_minax_val(Matrix *_mat, Matrix *_mat_position);

Matrix *M_logic_equal(Matrix *_mat, MATRIX_TYPE value);

Matrix *M_logic(Matrix *_mat_left, Matrix *_mat_right, int Operation);

Matrix *M_pmuldiv(Matrix *_mat_left, Matrix *_mat_right, int operation);

Matrix *M_setval(Matrix *_mat_ini, Matrix *_mat_val, Matrix *_mat_order, int model);

Matrix *M_numul_m(Matrix *_mat, Matrix *_mat_multi);

M_eigen_struct *M_eigen_max(Matrix *_mat);

int help(char *file_name);

int M_rank(Matrix *_mat);

int Etrans_free(Etrans_struct *_Etrans_);

Matrix *Hilbert(int order);

double M_cond(Matrix *_mat, int Setting);

Matrix ** M_eigen (Matrix *_mat);

Matrix * householder(Matrix * _x);

Matrix * M_householder(Matrix * _mat);

Matrix ** M_QR(Matrix * _mat);

Matrix * M_eigen_val(Matrix * _mat);

void progress_bar(int count, int total);

Matrix ** M_SVD(Matrix * _mat);

Matrix * M_pinv(Matrix * _mat, int _model_);

Matrix * M_Sample(Matrix* _mat_source, Matrix* _mat_sample, int mode);

#endif //_ST_MATRIX_H_