#define _USE_MATH_DEFINES
#include "PPH2016.h"
#include <stdio.h>
#include <math.h>
#include <tchar.h>
#include <time.h>  
#include <conio.h>
#include <math.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

//#include <vcl.h>

//#define SOLUCAOCCA2016

//*******************************PARÂMETROS************************************

static int numIntervaloAulaProfessor = 3; // intervalo entre o primeiro e o ultimo dia de aula do professor

										  // **************************** VARIÁVEIS GLOBAIS *****************************
										  // ------------ Auxiliares
ListaOfertas listaOfertas[NUM_OFERTA];
clock_t iH, fH, bstH;

// ------------ Problema
static Solucao vetSol[MAX_SOLUCAO];

// ------------ Dados de entrada
static Problema problema;
static Sala salas[NUM_SALA];
static Horario horarios[NUM_HORARIO];
static Professor professores[NUM_PROFESSOR];
static Turma turmas[NUM_TURMA];
static Oferta ofertas[NUM_OFERTA + NUM_BLOQ];
static Disciplina disciplinas[NUM_DISCIPLINA];
static TipoSala tipoSalas[NUM_TIPO_SALA];
static ListaCandidatos listaCandidatos[NUM_MAX_CANDIDATOS];

// ------------ Penalidades da FO
//-----Restrições Fortes----
static int w1 = 5000; // peso para conflito horario professor
static int w2 = 5000; // peso para conflito horario turma
static int w3 = 5000; // peso para conflito horario sala (sala atribuida a mais de uma turma no mesmo dia e horario)
static int w4 = 5000; // peso para turmas alocadas em sala de capacidade inferior
static int w5 = 5000; // peso para aulas alocadas em sala de tipo incompativel
static int w6 = 300;  // peso para aula de 3h alocadas fora do horario padrao
static int w7 = 5000; // peso para aulas alocadas fora do turno da oferta

					  //-----Restrições Fracas----
static int w8 = 10;   // peso para intervalo entre o primeiro e ultimo dia de aula do professor
static int w9 = 20;   // peso para janela de horarios nas grades das turmas
static int w10 = 4;   // peso para disciplinas fora do periodo preferencial
static int w11 = 600; // peso para aulas da mesma disciplina no mesmo dia para uma turma
static int w12 = 10;  // peso para professor que ministra aula a noite e no dia seguinte pela manha
static int w13 = 10;  // peso para aulas seguidas de nivel difícil
static int w14 = 10;  // peso para aulas de nivel dificil no ultimo horario
static int w15 = 500;  // peso para disciplinas do turno diurno com carga horaria par alocadas no primeiro horario do dia (7 horas)
static int w16 = 200;  //disciplinas alocadas entre o horário de almoço
static int w17 = 150;   //peso para disciplinas alocadas em horário fora do padrão. ex: 09:00 as 11:00, ou 14:30 as 16:30


						// ------------ Conflitos
static int CONF_PROFESSOR = 0;  	     	 //w1
static int CONF_TURMA = 0;      		    //w2
static int CONF_SALA = 0;       		    //w3
static int CONF_TURMA_CAPACIDADE = 0;   //w4
static int CONF_TP_INC_SALA = 0;        //w5
static int CONF_AULA_3H = 0;            //w6
static int CONF_FORA_TURNO_OFRT = 0;    //w7

static int CONF_INT_AULA_PROF = 0;      //w8
static int CONF_INT_HOR_TURMA = 0;      //w9
static int CONF_FORA_PER_PREF = 0;      //w10
static int CONF_AULAS_SEG_DISC = 0;     //w11
static int CONF_PROF_NOITE_MANHA = 0;   //w12
static int CONF_AULAS_SEG_DIF = 0;      //w13
static int CONF_AULA_DIF_ULT_HOR = 0;   //w14
static int CONF_AULA_PAR = 0;			    //w15
static int CONF_AULA_ALMOCO = 0;        //w16
static int CONF_FORA_HORA_PADRAO = 0;   //w17

										// ********************************* LEITURA DE DADOS ********************************

void lerDisciplinas() {
	FILE * arqDisciplina = fopen("arquivos2016/disciplinas.txt", "r");

	if (arqDisciplina != NULL) {
		problema.quantDisciplinas = NUM_DISCIPLINA;

		for (int i = 0; i < NUM_DISCIPLINA; i++) {
			fscanf(arqDisciplina, "%d", &disciplinas[i].id);
			fscanf(arqDisciplina, "%s", &disciplinas[i].codigo);
			fscanf(arqDisciplina, "%s", &disciplinas[i].nome);
			fscanf(arqDisciplina, "%d", &disciplinas[i].nivel);
		}
	}
	fclose(arqDisciplina);
}
// ------------------------------------------------------------------------------

void lerOfertas() {
	FILE * arqOferta = fopen("arquivos2016/ofertas.txt", "r");

	if (arqOferta != NULL) {

		for (int i = 0; i < NUM_OFERTA + NUM_BLOQ; i++) {
			fscanf(arqOferta, "%d", &ofertas[i].id);
			fscanf(arqOferta, "%d", &ofertas[i].idDisciplina);
			fscanf(arqOferta, "%d", &ofertas[i].numTurmas);
			//
			//	printf("%d %d \n", i,ofertas[i].numTurmas);

			ofertas[i].turmas = new int[ofertas[i].numTurmas];
			for (int j = 0; j < ofertas[i].numTurmas; j++) {
				fscanf(arqOferta, "%d", &ofertas[i].turmas[j]);
				//
				//	printf("%d \n", ofertas[i].turmas[j]);
			}
			fscanf(arqOferta, "%d", &ofertas[i].numVagas);
			fscanf(arqOferta, "%d", &ofertas[i].turno);
			fscanf(arqOferta, "%d", &ofertas[i].idProfessor);
			fscanf(arqOferta, "%d", &ofertas[i].idTipoSala);
			fscanf(arqOferta, "%d", &ofertas[i].ch);

		}
	}

	fclose(arqOferta);
}
// ------------------------------------------------------------------------------

void lerTurmas() {
	FILE * arqTurma = fopen("arquivos2016/turmas.txt", "r");

	if (arqTurma != NULL) {
		problema.quantTurmas = NUM_TURMA;
		for (int i = 0; i < NUM_TURMA; i++) {
			fscanf(arqTurma, "%d", &turmas[i].id);
			fscanf(arqTurma, "%s", &turmas[i].nome);
			fscanf(arqTurma, "%d", &turmas[i].periodo);
			fscanf(arqTurma, "%d", &turmas[i].turnoPreferencial);
		}
	}
	fclose(arqTurma);
}

// ------------------------------------------------------------------------------
void lerSalas() {
	FILE *arqSala = fopen("arquivos2016/locais.txt", "r");

	if (arqSala != NULL) {
		problema.quantSalas = NUM_SALA;
		for (int i = 0; i < NUM_SALA; i++) {
			fscanf(arqSala, "%d", &salas[i].id);
			fscanf(arqSala, "%s", &salas[i].predio);
			fscanf(arqSala, "%d", &salas[i].idTipoSala);
			fscanf(arqSala, "%d", &salas[i].numSala);
			fscanf(arqSala, "%d", &salas[i].capacidade);
		}
	}

	fclose(arqSala);

}

// ------------------------------------------------------------------------------
void lerTipoSalas() {
	FILE *arqSala = fopen("arquivos2016/tipos.txt", "r");

	if (arqSala != NULL) {
		problema.quantTipoSalas = NUM_TIPO_SALA;
		for (int i = 0; i < NUM_TIPO_SALA; i++) {
			fscanf(arqSala, "%d", &tipoSalas[i].id);
			fscanf(arqSala, "%s", &tipoSalas[i].nome);
		}
	}

	fclose(arqSala);
}

// ------------------------------------------------------------------------------
void lerHorarios() {
	FILE * arqHorario = fopen("arquivos2016/horarios.txt", "r");

	if (arqHorario != NULL) {
		problema.quantHorarios = NUM_HORARIO;
		for (int i = 0; i < NUM_HORARIO; i++) {
			fscanf(arqHorario, "%d", &horarios[i].id);
			fscanf(arqHorario, "%s", &horarios[i].inicio);
			fscanf(arqHorario, "%s", &horarios[i].fim);
			fscanf(arqHorario, "%d", &horarios[i].duracao);
		}
	}
	fclose(arqHorario);
}

// ------------------------------------------------------------------------------
void lerProfessores() {
	FILE * arqProfessor = fopen("arquivos2016/professores.txt", "r");

	if (arqProfessor != NULL) {
		problema.quantProfessores = NUM_PROFESSOR;
		for (int i = 0; i < NUM_PROFESSOR; i++) {
			fscanf(arqProfessor, "%d", &professores[i].id);
			fscanf(arqProfessor, "%s", &professores[i].nome);
		}
	}
	fclose(arqProfessor);
}

// ----------------------------------------------------------------------------

//---------------------Exportação de Parâmetros--------------------------------
void exportaParametros(Solucao *sol, double melhorTempo) {
	char pAux[10];
	char nome[100];

	strcpy(nome, "saidas/");
	//itoa(posR, pAux, 10);
	//strcat(nome, pAux);
	strcat(nome, "__parametros.txt");

	calcFO(sol);

	FILE *arqSala = fopen(nome, "wt");

	if (arqSala != NULL) {

		fprintf(arqSala, "Limites \n\n");
		fprintf(arqSala, "NUM_PROFESSOR = %d \n", NUM_PROFESSOR);
		fprintf(arqSala, "NUM_SALA = %d \n", NUM_SALA);
		fprintf(arqSala, "NUM_TIPO_SALA = %d \n", NUM_TIPO_SALA);
		fprintf(arqSala, "NUM_TURMA = %d \n", NUM_TURMA);
		fprintf(arqSala, "NUM_OFERTA = %d \n", NUM_OFERTA);
		fprintf(arqSala, "NUM_BLOQ = %d \n", NUM_BLOQ);
		fprintf(arqSala, "NUM_MAX_CANDIDATOS = %d \n", NUM_MAX_CANDIDATOS);
		fprintf(arqSala, "NUM_DISCIPLINA = %d \n", NUM_DISCIPLINA);
		fprintf(arqSala, "MAX_SOLUCAO = %d \n", MAX_SOLUCAO);
		fprintf(arqSala, "NUM_DIA = %d \n", NUM_DIA);
		fprintf(arqSala, "NUM_HORARIO = %d \n", NUM_HORARIO);

		fprintf(arqSala, "\nParametros do GRASP \n\n");
		fprintf(arqSala, "MAX_TEMPO = %f \n", MAX_TEMPO);
		fprintf(arqSala, "TEMP_INICIAL = %d \n", TEMP_INICIAL);
		fprintf(arqSala, "TAXA_RESFRIAMENTO = %f \n", TAXA_RESFRIAMENTO);
		fprintf(arqSala, "NUM_MAX_ITERACOES = %d \n", NUM_MAX_ITERACOES);
		fprintf(arqSala, "ALFA = %f \n", ALFA);
		//fprintf(arqSala, "PROB_MOVIMENTO = %f \n", PROB_MOVIMENTO);

		//	fprintf(arqSala, "INTERVALO_AULA_PROF = %d \n",numIntervaloAulaProfessor);

		fprintf(arqSala, "\n Penalidades da FO \n\n");
		fprintf(arqSala, "w1 = %d \n", w1);
		fprintf(arqSala, "w2 = %d \n", w2);
		fprintf(arqSala, "w3 = %d \n", w3);
		fprintf(arqSala, "w4 = %d \n", w4);
		fprintf(arqSala, "w5 = %d \n", w5);
		fprintf(arqSala, "w6 = %d \n", w6);
		fprintf(arqSala, "w7 = %d \n", w7);
		fprintf(arqSala, "w8 = %d \n", w8);
		fprintf(arqSala, "w9 = %d \n", w9);
		fprintf(arqSala, "w10 = %d \n", w10);
		fprintf(arqSala, "w11 = %d \n", w11);
		fprintf(arqSala, "w12 = %d \n", w12);
		fprintf(arqSala, "w13 = %d \n", w13);
		fprintf(arqSala, "w14 = %d \n", w14);
		fprintf(arqSala, "w15 = %d \n", w15);
		fprintf(arqSala, "w16 = %d \n", w16);
		fprintf(arqSala, "w17 = %d \n", w17);

		fprintf(arqSala, "\n Conflitos da FO \n\n");
		fprintf(arqSala, "CONF_PROFESSOR = %d \n", CONF_PROFESSOR);
		fprintf(arqSala, "CONF_TURMA = %d \n", CONF_TURMA);
		fprintf(arqSala, "CONF_SALA = %d \n", CONF_SALA);
		fprintf(arqSala, "CONF_TURMA_CAPACIDADE = %d \n", CONF_TURMA_CAPACIDADE);
		fprintf(arqSala, "CONF_TP_INC_SALA = %d \n", CONF_TP_INC_SALA);
		fprintf(arqSala, "CONF_AULA_3H = %d \n", CONF_AULA_3H);
		fprintf(arqSala, "CONF_FORA_TURNO_OFRT = %d \n", CONF_FORA_TURNO_OFRT);
		fprintf(arqSala, "CONF_INT_AULA_PROF = %d \n", CONF_INT_AULA_PROF);
		fprintf(arqSala, "CONF_INT_HOR_TURMA = %d \n", CONF_INT_HOR_TURMA);
		fprintf(arqSala, "CONF_FORA_PER_PREF = %d \n", CONF_FORA_PER_PREF);
		fprintf(arqSala, "CONF_AULAS_SEG_DISC = %d \n", CONF_AULAS_SEG_DISC);
		fprintf(arqSala, "CONF_PROF_NOITE_MANHA = %d \n", CONF_PROF_NOITE_MANHA);
		fprintf(arqSala, "CONF_AULAS_SEG_DIF = %d \n", CONF_AULAS_SEG_DIF);
		fprintf(arqSala, "CONF_AULA_DIF_ULT_HOR = %d \n", CONF_AULA_DIF_ULT_HOR);
		fprintf(arqSala, "CONF_AULA_PAR = %d \n", CONF_AULA_PAR);
		fprintf(arqSala, "CONF_AULA_ALMOCO = %d \n", CONF_AULA_ALMOCO);
		fprintf(arqSala, "CONF_FORA_HORA_PADRAO = %d \n", CONF_FORA_HORA_PADRAO);

		fprintf(arqSala, "\n Valor da FO: %ld    Tempo: %.2f\n ", sol->fo, melhorTempo);
	}
	fclose(arqSala);
}
//---------------------------------------------------------------------------------------------






void carregarHorarioBloqueadoSala(Solucao *sol) {
	//Carrega os  horários disponíveis de cada uma das 13 salas, com os horários das disciplinas de outros dep. previamente assinalados
	//E os valores de -1 e -2 significam, respectivamente, horário disponível e indisponível
	char pAux[10];
	char nome[100];
	FILE * arqSolucao;

	for (int s = 0; s < NUM_SALA; s++) {

		strcpy(nome, "arquivos2016/horarioSala/");
		itoa(s, pAux, 10);
		strcat(nome, pAux);
		strcat(nome, ".txt");

		arqSolucao = fopen(nome, "r");

		if (arqSolucao != NULL) {
			for (int h = 0; h < NUM_HORARIO; h++) {

				fscanf(arqSolucao, "%d", &(sol->gradeSala[s].gradeHorario[h][0])
				);
				fscanf(arqSolucao, "%d", &(sol->gradeSala[s].gradeHorario[h][1])
				);
				fscanf(arqSolucao, "%d", &(sol->gradeSala[s].gradeHorario[h][2])
				);
				fscanf(arqSolucao, "%d", &(sol->gradeSala[s].gradeHorario[h][3])
				);
				fscanf(arqSolucao, "%d", &(sol->gradeSala[s].gradeHorario[h][4])
				);
			}
		}
		fclose(arqSolucao);
	}
}


void iniciarSolucao(Solucao * sl) {
	// A solução é composta por 3 matrizes, sendo que cada matriz representa o resultado de acordo com a referência, podendo ser: por sala, por professor e/ou turma
	// Ficando mais fácil a visualização da solução de acordo com o interesse do usuário
	sl->fo = INT_MAX;
	for (int i = 0; i < NUM_SALA; i++) {
		for (int k = 0; k < NUM_DIA; k++) { // dias da semana
			for (int l = 0; l < NUM_HORARIO; l++) { // horarios
				sl->gradeSala[i].gradeHorario[l][k] = -1;
			}
		}
	}
	for (int i = 0; i < NUM_PROFESSOR; i++) {
		for (int k = 0; k < NUM_DIA; k++) { // dias da semana
			for (int l = 0; l < NUM_HORARIO; l++) { // horarios
				sl->gradeProfessor[i].gradeHorario[l][k] = -1;
			}
		}
	}
	for (int i = 0; i < NUM_TURMA; i++) {
		for (int k = 0; k < NUM_DIA; k++) { // dias da semana
			for (int l = 0; l < NUM_HORARIO; l++) { // horario
				sl->gradeTurma[i].gradeHorario[l][k] = -1;
			}
		}
	}

	carregarHorarioBloqueadoSala(sl); // carrega horarios limitados para as salas
}

/**
* Compara duas aulas em relação a dificuldade.
* O primeiro critério é indisponibilidade de horarios, seja por restrição inicial
* ou aulas da mesma grade de turma/professor. Se empatar nesse critério verifica-se qual
* disciplina está presente em mais grades de turma.
*/

void ordenaNumVagas(ListaOfertas *listaOfertas) {
	int i, j, min, max, aux1, aux2, aux3, aux4, aux5;
	max = NUM_OFERTA;

	for (i = 0; i < (max - 1); i++)
	{
		/* O minimo é o primeiro número não ordenado ainda */
		min = i;
		for (j = i + 1; j < max; j++)
		{
			/* Caso tenha algum numero menor ele faz a troca do minimo*/
			if (listaOfertas[j].numVagasOferta > listaOfertas[min].numVagasOferta)
			{
				min = j;
			}
		}
		/* Se o minimo for diferente do primeiro numero não ordenado ele faz a troca para ordena-los*/
		if (i != min)
		{
			aux1 = listaOfertas[i].idOferta;
			aux2 = listaOfertas[i].numVagasOferta;
			aux3 = listaOfertas[i].idTipoSalaOferta;
			aux4 = listaOfertas[i].chOferta;
			aux5 = listaOfertas[i].turnoOferta;

			listaOfertas[i].idOferta = listaOfertas[min].idOferta;
			listaOfertas[i].numVagasOferta = listaOfertas[min].numVagasOferta;
			listaOfertas[i].idTipoSalaOferta = listaOfertas[min].idTipoSalaOferta;
			listaOfertas[i].chOferta = listaOfertas[min].chOferta;
			listaOfertas[i].turnoOferta = listaOfertas[min].turnoOferta;

			listaOfertas[min].idOferta = aux1;
			listaOfertas[min].numVagasOferta = aux2;
			listaOfertas[min].idTipoSalaOferta = aux3;
			listaOfertas[min].chOferta = aux4;
			listaOfertas[min].turnoOferta = aux5;
		}
	}
}

int ordenaFo(ListaCandidatos *listaCandidatos) {
	int i, j, max, min, aux1, aux3, aux4, aux5, aux6;
	long aux2;

	//encontra o tamanho real da lista de candidatos, pegando o primeiro -1
	max = 0;
	for (int cont = 0; cont < NUM_MAX_CANDIDATOS; cont++)
	{
		if (listaCandidatos[cont].idOferta == -1)
		{
			break;
		}
		else
		{
			max++;
		}
	}


	for (i = 0; i < (max - 1); i++)
	{
		min = i;
		for (j = i + 1; j < max; j++)
		{
			if (listaCandidatos[j].fo < listaCandidatos[min].fo)
			{
				min = j;
			}
		}

		/* Se o minimo for diferente do primeiro numero não ordenado ele faz a troca para ordena-los*/
		if (i != min)
		{
			aux1 = listaCandidatos[i].idOferta;
			aux2 = listaCandidatos[i].fo;
			aux3 = listaCandidatos[i].sala;
			aux4 = listaCandidatos[i].dia;
			aux5 = listaCandidatos[i].hora;
			aux6 = listaCandidatos[i].ch;

			listaCandidatos[i].idOferta = listaCandidatos[min].idOferta;
			listaCandidatos[i].fo = listaCandidatos[min].fo;
			listaCandidatos[i].sala = listaCandidatos[min].sala;
			listaCandidatos[i].dia = listaCandidatos[min].dia;
			listaCandidatos[i].hora = listaCandidatos[min].hora;
			listaCandidatos[i].ch = listaCandidatos[min].ch;

			listaCandidatos[min].idOferta = aux1;
			listaCandidatos[min].fo = aux2;
			listaCandidatos[min].sala = aux3;
			listaCandidatos[min].dia = aux4;
			listaCandidatos[min].hora = aux5;
			listaCandidatos[min].ch = aux6;
		}
	}
	return(max);
}

bool verificaEspaco(Solucao * sol, int tam, int dia, int horario, int sala) {
	bool result = false;
	int i = 0;
	//'tam' representa a carga horária da oferta, que geralmente são de 2 horas por dia
	//verifica se o horário não ultrapassa o último horário (de índice 13), correspondente às 21:50
	if ((horario + tam - 1) < NUM_HORARIO) {
		for (i = 0; i < tam; i++) {
			result = true;
			if (sol->gradeSala[sala].gradeHorario[horario + i][dia] != -1) {
				result = false;
				break;
			}
		}
	}
	return result;
}

void criarSolucaoInicial(Solucao * sl) {
	int rndRC;
	int tamLC = 0; //Tamanho da lista de candidatos
	int tamLRC = 0; //Tamanho da lista restrita de candidatos
	long foFinal = 0; //Tamanho da F.O. final da Solução inicial

	for (int o = 0; o < NUM_OFERTA; o++) {
		listaOfertas[o].idOferta = ofertas[o].id;
		listaOfertas[o].numVagasOferta = ofertas[o].numVagas;
		listaOfertas[o].idTipoSalaOferta = ofertas[o].idTipoSala;
		listaOfertas[o].chOferta = ofertas[o].ch;
		listaOfertas[o].turnoOferta = ofertas[o].turno;
	}

	ordenaNumVagas(listaOfertas);

	//percorrendo todas as ofertas
	for (int i = 0; i < NUM_OFERTA; i++)
	{

		//inicializa a lista de candidatos para alocar de uma oferta
		int contCandidato = 0;
		for (int c = 0; c < NUM_MAX_CANDIDATOS; c++)
		{
			listaCandidatos[c].idOferta = -1;
			listaCandidatos[c].fo = -1;
			listaCandidatos[c].sala = -1;
			listaCandidatos[c].dia = -1;
			listaCandidatos[c].hora = -1;
			listaCandidatos[c].ch = -1;
		}

		// procuro por uma sala
		for (int s = 0; s < NUM_SALA; s++)
		{
			//verifica se a capacidade da sala é maior ou igual ao necessário para atender a oferta e se o tipo de sala é o mesmo
			if (salas[s].capacidade >= listaOfertas[i].numVagasOferta && salas[s].idTipoSala == listaOfertas[i].idTipoSalaOferta)
			{
				// no horario disponivel
				for (int d = 0; d < NUM_DIA; d++)
				{
					for (int h = 0; h < NUM_HORARIO; h++)
					{
						//verifica se o turno corresponde ao turno da oferta, sendo que se h for menor que 8, quer dizer que se trata do horário diurno
						if ((listaOfertas[i].turnoOferta == 1 && h <= 8) || (listaOfertas[i].turnoOferta == 2 && h > 8))
						{
							//evita que uma oferta de ch=2 possua horários de 16:30 as 17:30 e 18:20 as 20:00 e
							//evita que uma oferta de ch=2 possua horários de 11:00 as 12:00 e 13:30 as 14:30
							if ((!((h == 8) && (listaOfertas[i].chOferta + h > 8))) || (!((h == 6) && (listaOfertas[i].chOferta + h > 6))))
							{
								//verifica se o horário a ser alocado não possui oferta já alocada
								if (verificaEspaco(sl, listaOfertas[i].chOferta, d, h, s))
								{
									for (int p = 0; p < listaOfertas[i].chOferta; p++)
									{

										sl->gradeSala[s].gradeHorario[h + p][d] = listaOfertas[i].idOferta;

										//printf ("Sala: %d; Dia: %d; Hora: %d; Oferta: %d", s, d , h+p ,listaOfertas[i].idOferta);
										//printf ("\n");
									}
									//calcula a FO e armazena os valores que representam uma alocação de uma oferta, dentre uma lista de candidatos de alocação daquela oferta específica
									listaCandidatos[contCandidato].idOferta = listaOfertas[i].idOferta;
									calcFO(sl);
									listaCandidatos[contCandidato].fo = sl->fo;
									listaCandidatos[contCandidato].sala = s;
									listaCandidatos[contCandidato].dia = d;
									listaCandidatos[contCandidato].hora = h;
									listaCandidatos[contCandidato].ch = listaOfertas[i].chOferta;

									//volta o valor da simulação para -1
									for (int p = 0; p < listaOfertas[i].chOferta; p++)
									{
										sl->gradeSala[s].gradeHorario[h + p][d] = -1;
									}

									contCandidato++;
									//printf ("\n");
								}
							}
						}
					}
				}
			}
		}
		//Ordena a lista de candidatos
		tamLC = ordenaFo(listaCandidatos);
		//Realizar o corte na lista de candidatos baseado no alfa e escolher randomicamente uma alocação dentro da lista restrita feita pelo corte
		if (ALFA == 0)
		{
			rndRC = 0;
		}
		else
		{
			tamLRC = (int)(tamLC * ALFA);
			// rndRC em um range de 0 até (tamLRC-1)
			srand(time(NULL));
			rndRC = rand() % tamLRC;
		}

		//Depois disto, aí sim aloca-se a oferta definitivamente
		for (int p = 0; p < listaCandidatos[rndRC].ch; p++)
		{

			sl->gradeSala[listaCandidatos[rndRC].sala].gradeHorario[listaCandidatos[rndRC].hora + p][listaCandidatos[rndRC].dia] = listaCandidatos[rndRC].idOferta;
			//printf("Melhor FO: %d ; Oferta: %d ; Sala: %d ; dia: %d  ; h: %d ",listaCandidatos[rndRC].fo,listaCandidatos[rndRC].idOferta, listaCandidatos[rndRC].sala, listaCandidatos[rndRC].dia, listaCandidatos[rndRC].hora + p);
			//printf ("\n");
		}
	}
	calcFO(sl);
	//foFinal = sl->fo;
	//printf("%d",foFinal);
	//printf ("\n");
	//exportaSala(&sl[0],1);
	//exportaTurma(&sl[0],1);
	//exportaProfessor(&sl[0],1);
}

// ********************************* FO **********************************\\
// *********************************    **********************************\\

void calcFO(Solucao *sol) {
	int idOferta, idOfertaAnt;
	sol->fo = 0;
	iniciarVariaveis();
	//Gera tabelas de professor e turma, para poder encontrar demais conflitos
	gerarTabelasPeT(sol);

	// conflito de professor e turma
	for (int d = 0; d < NUM_DIA; d++)
	{
		for (int h = 0; h < NUM_HORARIO; h++)
		{
			for (int s = 0; s < NUM_SALA; s++)
			{
				for (int s1 = (s + 1); s1 < NUM_SALA; s1++)
				{
					idOferta = sol->gradeSala[s].gradeHorario[h][d];
					idOfertaAnt = sol->gradeSala[s].gradeHorario[h - 1][d];
					if (sol->gradeSala[s].gradeHorario[h][d] > -1 && sol->gradeSala[s1].gradeHorario[h][d] > -1)
					{
						// if (idOferta != idOfertaAnt){
						// conflito de horario professor
						if (ofertas[sol->gradeSala[s].gradeHorario[h][d]].idProfessor == ofertas[sol->gradeSala[s1].gradeHorario[h][d]].idProfessor)
						{
							CONF_PROFESSOR++;
						}

						// conflito de horario turma
						//o maior número de turmas em uma oferta encontrado nesta modelagem foi de 3 turmas, ou seja, t varia no máximo de 0 a 2
						for (int t = 0; t < ofertas[sol->gradeSala[s].gradeHorario[h][d]].numTurmas; t++)
						{
							//verifica se uma turma está alocada em duas salas diferentes, no mesmo dia e hora
							if (ofertas[sol->gradeSala[s].gradeHorario[h][d]].turmas[t] == ofertas[sol->gradeSala[s1].gradeHorario[h][d]].turmas[t])
							{
								CONF_TURMA++;
							}
						}
						//}
					}
				}
			}
		}
	}
	int idProf, idProfAnt;

	for (int d = 0; d < NUM_DIA; d++)
	{
		for (int h = 0; h < NUM_HORARIO; h++)
		{

			for (int p = 0; p < NUM_PROFESSOR; p++)
			{
				for (int p1 = (p + 1); p1 < NUM_PROFESSOR; p1++)
				{
					if (sol->gradeProfessor[p].gradeHorario[h][d] > -1 && sol->gradeProfessor[p1].gradeHorario[h][d] > -1)
					{
						idProf = sol->gradeProfessor[p].gradeHorario[h][d];
						idProfAnt = sol->gradeProfessor[p].gradeHorario[h - 1][d];
						//if (idProf != idProfAnt)
						//{
						if (sol->gradeProfessor[p].gradeHorario[h][d] == sol->gradeProfessor[p1].gradeHorario[h][d])
						{
							CONF_SALA++;
						}
						//}
					}
				}
			}
		}
	}

	for (int d = 0; d < NUM_DIA; d++)
	{
		for (int h = 0; h < NUM_HORARIO; h++)
		{
			for (int t = 0; t < NUM_TURMA; t++)
			{
				for (int t1 = (t + 1); t1 < NUM_TURMA; t1++)
				{
					if (sol->gradeTurma[t].gradeHorario[h][d] > -1 && sol->gradeTurma[t1].gradeHorario[h][d] > -1)
					{
						if (sol->gradeTurma[t].gradeHorario[h][d] == sol->gradeTurma[t1].gradeHorario[h][d])
						{
							//verifica se o numero de turmas da oferta é maior que 1, para escapar de conflitos de sala quando uma oferta pode possuir 2 turmas ao mesmo tempo
							if (!(ofertas[sol->gradeSala[sol->gradeTurma[t].gradeHorario[h][d]].gradeHorario[h][d]].numTurmas > 1))
							{
								CONF_SALA++;
							}
						}
					}
				}
			}
		}
	}
	sol->fo = w1 * CONF_PROFESSOR + w2 * CONF_TURMA + w3 * CONF_SALA + confAula(sol) + confProfessor(sol) + confAula3h(sol);
}

void gerarTabelasPeT(Solucao * sol) {
	int s, h, d, t;
	iniciaProfessorTurma(sol);

	for (s = 0; s < NUM_SALA; s++)
	{
		for (h = 0; h < NUM_HORARIO; h++)
		{
			for (d = 0; d < NUM_DIA; d++)
			{
				//verifica se já existe alguma oferta alocada
				if (sol->gradeSala[s].gradeHorario[h][d] > -1)
				{
					// aloca a sala para a grade do professor, sendo reconhecido pelo id que consta na oferta
					sol->gradeProfessor[ofertas[sol->gradeSala[s].gradeHorario[h][d]].idProfessor].gradeHorario[h][d] = s;
					//este for serve para contornar ofertas que tenham mais de uma turma
					//para o número de turmas, que pode variar de 1 até 3, de uma oferta alocada em uma sala, em um determinado dia/hora
					for (t = 0; t < ofertas[sol->gradeSala[s].gradeHorario[h][d]].numTurmas; t++)
					{
						//aloca a sala para uma turma (nota-se que será alocado uma mesma sala para varias turmas diferentes, gerando conflitos de sala)
						sol->gradeTurma[ofertas[sol->gradeSala[s].gradeHorario[h][d]].turmas[t]].gradeHorario[h][d] = s;
					}
				}
			}
		}
	}
}


//Quantifica as restrições 9, 11, 13, 14
long confAula(Solucao *sol) {

	int primeiroHorario, ultimoHorario, primeiroDia, ultimoDia = -1;
	int idOferta1, idOferta2, idOfertaAux, idOfertaAnt, idOfertaAnt2, ch1 = -1;
	bool encontrou = false;

	// aulas de nível difícil
	for (int t = 0; t < NUM_TURMA; t++)
	{
		primeiroDia = -1;
		for (int d = 0; d < NUM_DIA; d++)
		{
			primeiroHorario = -1;
			for (int h = 0; h < NUM_HORARIO; h++)
			{
				//se existir oferta alocada no horário e dia da grade da turma
				if (sol->gradeTurma[t].gradeHorario[h][d] > -1)
				{
					// intervalos na grade de horarios
					//primeiroHorario captura a primeira hora do dia em que existe uma disciplina alocada
					if (primeiroHorario == -1)
					{
						primeiroHorario = h;
					}
					//no último incremento, ultimoHorario receberá a última hora utilizada
					ultimoHorario = h;

					if (primeiroDia == -1)
					{
						primeiroDia = d;
					}
					ultimoDia = d;

					//se h+1 não extrapola a quantidade de horários existentes
					if ((h + 1) <= NUM_HORARIO)
					{

						//oferta alocada na sala X proveniente do id de Sala da grade da Turma
						idOferta1 = sol->gradeSala[sol->gradeTurma[t].gradeHorario[h][d]].gradeHorario[h][d];
						idOfertaAnt = sol->gradeSala[sol->gradeTurma[t].gradeHorario[h - 1][d]].gradeHorario[h - 1][d];
						ch1 = ofertas[idOferta1].ch;
						if (idOferta1 != idOfertaAnt)
						{
							//se h + a ch da oferta não passar de 13 horários e se, o horário + a ch da oferta, da grade da turma não estiver vazio
							if (h + ch1 <= NUM_HORARIO && sol->gradeTurma[t].gradeHorario[h + ch1][d] > -1)
							{
								//pega o id da oferta posterior da grade de sala, com base na grade de turma, onde contem o id de sala
								idOferta2 = sol->gradeSala[sol->gradeTurma[t].gradeHorario[h + ch1][d]].gradeHorario[h + ch1][d];
								idOfertaAnt2 = sol->gradeSala[sol->gradeTurma[t].gradeHorario[h + ch1 - 1][d]].gradeHorario[h + ch1 - 1][d];
								if (idOferta2 != idOfertaAnt2)
								{
									// aulas seguidas com nivel dificil   (1 - dificil)
									if (disciplinas[ofertas[idOferta1].idDisciplina].nivel == 1 && disciplinas[ofertas[idOferta2].idDisciplina].nivel == 1)
									{
										CONF_AULAS_SEG_DIF++;  //w13 - 2 ofertas de aulas difíceis seguidas no mesmo dia
									}
								}
							}
						}
					}

					//// aula  da mesma disciplina no mesmo dia
					idOferta1 = sol->gradeSala[sol->gradeTurma[t].gradeHorario[h][d]].gradeHorario[h][d];
					ch1 = ofertas[idOferta1].ch;
					idOfertaAux = sol->gradeSala[sol->gradeTurma[t].gradeHorario[h - 1][d]].gradeHorario[h - 1][d];

					//evita penalização redundante
					if (ch1 == 2 && idOferta1 != idOfertaAux)
					{
						//verifica entre os horários do dia atual se existe outra oferta com a mesma disciplina
						for (int a = h + ch1; a < NUM_HORARIO; a++)
						{
							if (sol->gradeTurma[t].gradeHorario[a][d] > -1)
							{
								idOferta2 = sol->gradeSala[sol->gradeTurma[t].gradeHorario[a][d]].gradeHorario[a][d];
								if (ofertas[idOferta1].idDisciplina == ofertas[idOferta2].idDisciplina)
								{
									CONF_AULAS_SEG_DISC++;  //w11 - 2 ofertas da mesma disciplina em um mesmo dia
									a = ofertas[idOferta2].ch + a;
								}
							}
						}
					}

					// aula dificil no ultimo horario (16h30 a 17h30 ou 21h00 a 21h50 ou 21h50 a 22h40)
					if (h == 8 || h > 11)
					{
						if ((disciplinas[ofertas[(sol->gradeSala[sol->gradeTurma[t].gradeHorario[h][d]].gradeHorario[h][d])].idDisciplina].nivel) == 1)
						{
							CONF_AULA_DIF_ULT_HOR++;  //w14 - aulas de nível difícil no último horário
						}
					}
				}
			}
			// horarios vagos no mesmo dia entre as disciplinas
			if (primeiroHorario > -1)
			{
				for (int h = primeiroHorario; h < ultimoHorario; h++)
				{
					if (sol->gradeTurma[t].gradeHorario[h][d] == -1)
					{
						CONF_INT_HOR_TURMA++;  //w9 - Contabiliza janelas de horários das turmas
					}
				}
			}

		}
		// Janelas de dias sem aula durante a semana, para uma turma
		if (primeiroDia > -1)
		{
			for (int d = primeiroDia; d <= ultimoDia; d++)
			{
				//percorre todos os horários de um dia, caso este dia não tenha no mínimo 1 horário ocupado, é contabilizada  janela de dia sem aula
				for (int h = 0; h < NUM_HORARIO; h++)
				{
					if (sol->gradeTurma[t].gradeHorario[h][d] > -1)
					{
						encontrou = true;
					}
				}
				if (encontrou)
				{
					encontrou = false;
				}
				else
				{
					CONF_INT_HOR_TURMA++; // w9 - Contabiliza janelas de horários entre dias, sem aula, de uma turma
				}
			}
		}
	}

	return((w9 * CONF_INT_HOR_TURMA) + (w11 * CONF_AULAS_SEG_DISC) + (w13 * CONF_AULAS_SEG_DIF) + (w14 * CONF_AULA_DIF_ULT_HOR));
}

//Quantifica as restrições 8 e 12
long confProfessor(Solucao *sol) {
	int diaInicio = -1;
	int diaFim = -1;
	bool encontrou = false;

	// intervalo de dia do professor
	for (int p = 0; p < NUM_PROFESSOR; p++)
	{
		diaInicio = -1;
		diaFim = -1;
		for (int d = 0; d < NUM_DIA; d++)
		{
			for (int h = 0; h < NUM_HORARIO; h++)
			{
				if (sol->gradeProfessor[p].gradeHorario[h][d] > -1)
				{
					if (diaInicio == -1)
					{
						diaInicio = d;
					}
					diaFim = d;

					// conflito professor que da aula no ultimo horario a noite e no outro dia cedo
					// a partir das 20h
					if (h >= 11)
					{
						// verifica se não é sexta-feira
						if ((d + 1) < NUM_DIA)
						{
							// se existe o proximo dia, para os horários 0, 1 e 2 (referentes a manhã)
							for (int l = 0; l < 3; l++)
							{
								if (sol->gradeProfessor[p].gradeHorario[l][d + 1] > -1)
								{
									// se houver aula de manhã para o professor, o l é incrementado de modo que já saia do laço caso a ch seja 2
									l += ofertas[(sol->gradeSala[sol->gradeProfessor[p].gradeHorario[l][d + 1]].gradeHorario[l][d + 1])].ch;
									CONF_PROF_NOITE_MANHA++; //w12 - Contabiliza aulas ministradas por um professor quando temos o último horário a noite e alguma aula em um dos três primeiros horários da manhã do dia seguinte
								}
							}
						}
					}
					// incrementa a hora
					h += ofertas[sol->gradeSala[sol->gradeProfessor[p].gradeHorario[h][d]].gradeHorario[h][d]].ch;
				}
			}
		}


		if (((diaFim - diaInicio) + 1) > numIntervaloAulaProfessor)
		{
			CONF_INT_AULA_PROF++; //w8 - Contabiliza o tamanho do intervalo entre aulas de um professor, em dias
		}


		if (diaInicio > -1)
		{
			for (int dia = diaInicio; dia <= diaFim; dia++)
			{
				//percorre todos os horários de um dia, caso este dia não tenha no mínimo 1 horário ocupado, é contabilizada  janela de dia sem aula
				for (int hr = 0; hr < NUM_HORARIO; hr++)
				{
					if (sol->gradeProfessor[hr].gradeHorario[hr][dia] > -1)
					{
						encontrou = true;
					}
				}
				if (encontrou)
				{
					encontrou = false;
				}
				else
				{
					CONF_INT_AULA_PROF++; //w8 - Contabiliza o tamanho do intervalo entre aulas de um professor, em dias
				}
			}
		}
	}

	return((w8 * CONF_INT_AULA_PROF) + (w12 * CONF_PROF_NOITE_MANHA));
}


//Quantifica as restrições 4, 5, 6, 7, 10 e 15
long confAula3h(Solucao *sol) {
	int idOfertaAnt = -1;
	int idOferta = -1;
	int t = 0;
	// aulas de 3h fora do primeiro horario,
	for (int s = 0; s < NUM_SALA; s++)
	{
		for (int d = 0; d < NUM_DIA; d++)
		{
			// periodo diurno
			for (int h = 3; h < 9; h++)
			{
				if (sol->gradeSala[s].gradeHorario[h][d] > -1)
				{
					idOferta = sol->gradeSala[s].gradeHorario[h][d];
					idOfertaAnt = sol->gradeSala[s].gradeHorario[h - 1][d];
					if (ofertas[sol->gradeSala[s].gradeHorario[h][d]].ch == 3)
					{
						CONF_AULA_3H++;
					}
				}
			}

			//noturno
			for (int h = 9; h < 11; h++)
			{
				idOferta = sol->gradeSala[s].gradeHorario[h][d];
				idOfertaAnt = sol->gradeSala[s].gradeHorario[h - 1][d];
				if (sol->gradeSala[s].gradeHorario[h][d] > -1)
				{
					if (ofertas[sol->gradeSala[s].gradeHorario[h][d]].ch == 3)
					{
						CONF_AULA_3H++;
					}
				}
			}

			/* periodo noturno
			int numOferta = -1;
			int contOferta = 0;
			for (int h = 10; h < 13; h++)
			{
			idOferta = sol->gradeSala[s].gradeHorario[h][d];
			idOfertaAnt = sol->gradeSala[s].gradeHorario[h-1][d];
			if (sol->gradeSala[s].gradeHorario[h][d] > -1)
			{
			if (ofertas[sol->gradeSala[s].gradeHorario[h][d]].ch == 3)
			{
			if(numOferta ==  sol->gradeSala[s].gradeHorario[h][d])
			{
			contOferta++;
			}
			numOferta =  sol->gradeSala[s].gradeHorario[h][d];
			}
			}
			}

			if(contOferta == 2 && idOferta != idOfertaAnt)
			{
			CONF_AULA_3H++;
			}
			*/
			// conf de capacidade e tipo da sala
			for (int h = 0; h < NUM_HORARIO; h++)
			{
				idOferta = sol->gradeSala[s].gradeHorario[h][d];
				idOfertaAnt = sol->gradeSala[s].gradeHorario[h - 1][d];
				if (sol->gradeSala[s].gradeHorario[h][d] > -1)
				{
					// ------------------------------------------
					// turma maior que a capacidade da sala
					if (ofertas[sol->gradeSala[s].gradeHorario[h][d]].numVagas > salas[s].capacidade && idOferta != idOfertaAnt)
					{
						CONF_TURMA_CAPACIDADE++;
					}
					// ------------------------------------------
					// turma alocada em tipo incompativel de sala
					if (ofertas[sol->gradeSala[s].gradeHorario[h][d]].idTipoSala != salas[s].idTipoSala && idOferta != idOfertaAnt)
					{
						CONF_TP_INC_SALA++;
					}
					// ------------------------------------------
					// fora do turno da oferta
					if (ofertas[sol->gradeSala[s].gradeHorario[h][d]].turno == 1)
					{
						// turno noturno
						if ((h >= 9))
						{
							CONF_FORA_TURNO_OFRT++;  //w7
						}
					}
					else
					{
						if (h < 9)
						{
							CONF_FORA_TURNO_OFRT++;      //w7
						}
					}
					// -----------------------------------------
					// disciplina ch par no primeiro horario
					if (h == 0)
					{
						if ((ofertas[sol->gradeSala[s].gradeHorario[h][d]].ch % 2) == 0)
						{
							CONF_AULA_PAR++;
						}
					}
					//-------------------------------------------

					// ------------------------------------------
					idOfertaAnt = sol->gradeSala[s].gradeHorario[h - 1][d];
					idOferta = sol->gradeSala[s].gradeHorario[h][d];

					// fora do turno preferencial
					if (ofertas[sol->gradeSala[s].gradeHorario[h][d]].turno == 1)
					{

						// manha
						if ((h < 5))
						{
							// para cada turma da oferta
							for (t = 0; t < ofertas[sol->gradeSala[s].gradeHorario[h][d]].numTurmas; t++)
							{
								if (turmas[ofertas[sol->gradeSala[s].gradeHorario[h][d]].turmas[t]].turnoPreferencial >= 1)
								{
									if (idOferta != idOfertaAnt)
									{
										CONF_FORA_PER_PREF++; //w10 - Fora do turno preferencial
									}
								}
							}

						} // tarde
						else if ((h >= 5) && (h < 9))
						{
							for (t = 0; t < ofertas[sol->gradeSala[s].gradeHorario[h][d]].numTurmas; t++)
							{
								if (turmas[ofertas[sol->gradeSala[s].gradeHorario[h][d]].turmas[t]].turnoPreferencial != 1)
								{
									if (idOferta != idOfertaAnt)
									{
										CONF_FORA_PER_PREF++; //w10 - Fora do turno preferencial
									}
								}
							}
						}
					}
					else
					{
						// turno noturno
						if ((h >= 9))
						{
							for (t = 0; t < ofertas[sol->gradeSala[s].gradeHorario[h][d]].numTurmas; t++)
							{
								if (turmas[ofertas[sol->gradeSala[s].gradeHorario[h][d]].turmas[t]].turnoPreferencial < 2)
								{
									if (idOferta != idOfertaAnt)
									{
										CONF_FORA_PER_PREF++; //w10 - Fora do turno preferencial
									}
								}
							}
						}
					}
				}
			}
		}
	}

	for (int s = 0; s < NUM_SALA; s++)
	{
		for (int d = 0; d < NUM_DIA; d++)
		{
			for (int h = 0; h < NUM_HORARIO; h++)
			{
				// oferta dividida no horario do almoco
				if ((sol->gradeSala[s].gradeHorario[h][d] > -1) && (h == 4))
				{
					if (sol->gradeSala[s].gradeHorario[h][d] == sol->gradeSala[s].gradeHorario[h + 1][d])
					{
						CONF_AULA_ALMOCO++;  //w16
					}
				}

				if ((sol->gradeSala[s].gradeHorario[h][d] > -1) && (h == 2 || h == 6 || h == 10 || h == 12))
				{

					if ((ofertas[sol->gradeSala[s].gradeHorario[h][d]].ch == 2) && (sol->gradeSala[s].gradeHorario[h][d] == sol->gradeSala[s].gradeHorario[h + 1][d]))
					{
						CONF_FORA_HORA_PADRAO++;  //w17
					}
					if ((ofertas[sol->gradeSala[s].gradeHorario[h][d]].ch == 4) && (sol->gradeSala[s].gradeHorario[h][d] == sol->gradeSala[s].gradeHorario[h + 3][d]))
					{
						CONF_FORA_HORA_PADRAO++;  //w17
					}
				}
				if (sol->gradeSala[s].gradeHorario[h][d] > -1)
				{
					if (ofertas[sol->gradeSala[s].gradeHorario[h][d]].ch == 1)
					{
						if (h < 12 && ofertas[sol->gradeSala[s].gradeHorario[h][d]].turno == 2)
						{
							CONF_FORA_HORA_PADRAO++;  //w17
						}
					}
				}

			}
		}
	}


	return((w6 * CONF_AULA_3H) + (w4 * CONF_TURMA_CAPACIDADE) +
		(w5 * CONF_TP_INC_SALA) + (w7 * CONF_FORA_TURNO_OFRT) +
		(w10 * CONF_FORA_PER_PREF) + (w15 * CONF_AULA_PAR) + (w16 * CONF_AULA_ALMOCO) + (w17 * CONF_FORA_HORA_PADRAO));
}



void iniciaProfessorTurma(Solucao * sl) {
	for (int i = 0; i < NUM_PROFESSOR; i++) {
		for (int k = 0; k < NUM_DIA; k++) {
			for (int l = 0; l < NUM_HORARIO; l++) {
				sl->gradeProfessor[i].gradeHorario[l][k] = -1;
			}
		}
	}

	for (int i = 0; i < NUM_TURMA; i++) {
		for (int k = 0; k < NUM_DIA; k++) {
			for (int l = 0; l < NUM_HORARIO; l++) {
				sl->gradeTurma[i].gradeHorario[l][k] = -1;
			}
		}
	}

}


void iniciarVariaveis() {
	CONF_PROFESSOR = 0;  		   //w1
	CONF_TURMA = 0;      		   //w2
	CONF_SALA = 0;       		   //w3
	CONF_TURMA_CAPACIDADE = 0;    //w4
	CONF_TP_INC_SALA = 0;         //w5
	CONF_AULA_3H = 0;             //w6
	CONF_FORA_TURNO_OFRT = 0;     //w7
	CONF_INT_AULA_PROF = 0;       //w8
	CONF_INT_HOR_TURMA = 0;       //w9
	CONF_FORA_PER_PREF = 0;       //w10
	CONF_AULAS_SEG_DISC = 0;      //w11
	CONF_PROF_NOITE_MANHA = 0;    //w12
	CONF_AULAS_SEG_DIF = 0;       //w13
	CONF_AULA_DIF_ULT_HOR = 0;    //w14
	CONF_AULA_PAR = 0;			   //w15
	CONF_AULA_ALMOCO = 0;         //w16
	CONF_FORA_HORA_PADRAO = 0;    //w17
}


//cira um arquivo para a grade de salas
void exportaSala(Solucao * ss, int posR) {

	int numOfertas = 0;

	static char semana[7][4] = {
		"SEG", "TER", "QUA", "QUI", "SEX", "SAB", "DOM"
	};
	// escrevendo horario das salas
	for (int i = 0; i < NUM_SALA; i++) {

		char pAux[10];
		char nome[100];

		strcpy(nome, "saidas/salas/");
		strcat(nome, "Sala ");
		itoa(salas[i].id, pAux, 10); //converte um inteiro em string.. transforma posR em string e armazena em pAux, sendo 10 o valor base de posR (base decimal)
		strcat(nome, pAux);
		strcat(nome, ".csv");

		FILE *arqSala = fopen(nome, "wt");

		if (arqSala != NULL)
		{
			fprintf(arqSala, ";");
			for (int k = 0; k < NUM_DIA; k++)
			{
				fprintf(arqSala, "%s;", semana[k]);
			}
			fprintf(arqSala, "\n");

			for (int j = 0; j < NUM_HORARIO; j++)
			{
				for (int k = 0; k < NUM_DIA; k++)
				{
					if (k == 0)
					{
						fprintf(arqSala, "%s;", horarios[j].inicio);
					}
					if (ss->gradeSala[i].gradeHorario[j][k] <= -1)
					{
						fprintf(arqSala, ";");
					}
					else
					{
						itoa(ofertas[ss->gradeSala[i].gradeHorario[j][k]].id, pAux, 10);
						fprintf(arqSala, "%s ", pAux);
						fprintf(arqSala, "%s ", disciplinas[ofertas[ss->gradeSala[i].gradeHorario[j][k]].idDisciplina].nome);
						for (int l = 0; l < ofertas[ss->gradeSala[i].gradeHorario[j][k]].numTurmas; l++)
						{
							fprintf(arqSala, "%s ", turmas[ofertas[ss->gradeSala[i].gradeHorario[j][k]].turmas[l]].nome);
							itoa(turmas[ofertas[ss->gradeSala[i].gradeHorario[j][k]].turmas[l]].periodo, pAux, 10);
							fprintf(arqSala, "%s ", pAux);
						}
						fprintf(arqSala, "%s ", professores[ofertas[ss->gradeSala[i].gradeHorario[j][k]].idProfessor].nome);
						itoa(salas[i].capacidade, pAux, 10);
						fprintf(arqSala, "S-%s ", pAux);
						itoa(ofertas[ss->gradeSala[i].gradeHorario[j][k]].numVagas, pAux, 10);
						fprintf(arqSala, "O-%s ;", pAux);

						numOfertas++;

					}
				}
				fprintf(arqSala, "\n");
			}
		}
		fclose(arqSala);
	}
	printf("%d ", numOfertas / 2);
}

void exportaTurma(Solucao * ss, int posR) {
	static char semana[7][4] = {
		"SEG", "TER", "QUA", "QUI", "SEX", "SAB", "DOM"
	};
	// escrevendo horario das turmas
	for (int i = 0; i < NUM_TURMA; i++) {

		char pAux[10];
		char nome[100];

		strcpy(nome, "saidas/turmas/");
		strcat(nome, "Turma ");
		itoa(turmas[i].id, pAux, 10);
		strcat(nome, pAux);
		strcat(nome, ".csv");

		FILE *arqSala = fopen(nome, "wt");

		if (arqSala != NULL) {
			fprintf(arqSala, ";");
			for (int k = 0; k < NUM_DIA; k++) {
				fprintf(arqSala, "%s;", semana[k]);
			}
			fprintf(arqSala, "\n");

			for (int j = 0; j < NUM_HORARIO; j++) {
				for (int k = 0; k < NUM_DIA; k++) {
					if (k == 0) {
						fprintf(arqSala, "%s;", horarios[j].inicio);
					}
					if (ss->gradeTurma[i].gradeHorario[j][k] == -1) {
						fprintf(arqSala, ";");
					}
					else {
						fprintf(arqSala, "%s %s %s",
							disciplinas[ofertas[ss->gradeSala[ss->gradeTurma[i].gradeHorario[j][k]].gradeHorario[j][k]]
							.idDisciplina].codigo,
							disciplinas[ofertas[ss->gradeSala[ss->gradeTurma[i].gradeHorario[j][k]].gradeHorario[j][k]]
							.idDisciplina].nome,
							salas[ss->gradeTurma[i].gradeHorario[j][k]].predio);

						char pAux[10];
						itoa(salas[ss->gradeTurma[i].gradeHorario[j][k]].numSala, pAux, 10);
						fprintf(arqSala, "-%s ", pAux);

						fprintf(arqSala, "%s ",
							professores[ofertas[ss->gradeSala[ss->gradeTurma[i].gradeHorario[j][k]].gradeHorario[j][k]]
							.idProfessor].nome);

						itoa(ofertas[ss->gradeSala[ss->gradeTurma[i].gradeHorario[j][k]].gradeHorario[j][k]].numVagas, pAux, 10);
						fprintf(arqSala, "Vg-%s ;", pAux);
					}

				}
				fprintf(arqSala, "\n");

			}
		}
		fclose(arqSala);
	}

}

void exportaProfessor(Solucao * ss, int posR) {
	static char semana[7][4] = {
		"SEG", "TER", "QUA", "QUI", "SEX", "SAB", "DOM"
	};
	// escrevendo horario das turmas
	for (int i = 0; i < NUM_PROFESSOR; i++) {

		char pAux[10];
		char nome[100];

		strcpy(nome, "saidas/professores/");
		itoa(professores[i].id, pAux, 10);
		strcat(nome, pAux);
		strcat(nome, professores[i].nome);
		strcat(nome, ".csv");

		FILE *arqSala = fopen(nome, "wt");

		if (arqSala != NULL) {
			fprintf(arqSala, ";");
			for (int k = 0; k < NUM_DIA; k++) {
				fprintf(arqSala, "%s;", semana[k]);
			}
			fprintf(arqSala, "\n");

			for (int j = 0; j < NUM_HORARIO; j++) {
				for (int k = 0; k < NUM_DIA; k++) {
					if (k == 0) {
						fprintf(arqSala, "%s;", horarios[j].inicio);
					}
					if (ss->gradeProfessor[i].gradeHorario[j][k] == -1) {
						fprintf(arqSala, ";");
					}
					else {
						fprintf(arqSala, "%s %s;",
							disciplinas[ofertas[ss->gradeSala[ss->gradeProfessor[i].gradeHorario[j][k]].gradeHorario[j][k]]
							.idDisciplina].codigo,
							disciplinas[ofertas[ss->gradeSala[ss->gradeProfessor[i].gradeHorario[j][k]].gradeHorario[j][k]].idDisciplina].nome);
					}

				}
				fprintf(arqSala, "\n");

			}
		}
		fclose(arqSala);
	}

}

void clonarSolucao(Solucao *original, Solucao *clone) {
	int i, k, l;
	clone->fo = original->fo;

	for (i = 0; i < NUM_SALA; i++)
	{
		for (k = 0; k < NUM_DIA; k++)
		{
			for (l = 0; l < NUM_HORARIO; l++)
			{
				clone->gradeSala[i].gradeHorario[l][k] = original->gradeSala[i].gradeHorario[l][k];
			}
		}
	}
	for (i = 0; i < NUM_PROFESSOR; i++)
	{
		for (k = 0; k < NUM_DIA; k++)
		{
			for (l = 0; l < NUM_HORARIO; l++)
			{
				clone->gradeProfessor[i].gradeHorario[l][k] = original->gradeProfessor[i].gradeHorario[l][k];
			}
		}
	}
	for (i = 0; i < NUM_TURMA; i++)
	{
		for (k = 0; k < NUM_DIA; k++)
		{
			for (l = 0; l < NUM_HORARIO; l++)
			{
				clone->gradeTurma[i].gradeHorario[l][k] = original->gradeTurma[i].gradeHorario[l][k];
			}
		}
	}
}

void geraSwap(Solucao *solucaoVizinho) {
	//random entre as posições que possuem aula
	int pos1Hora, pos2Hora, pos1Dia, pos2Dia, pos1Sala, pos2Sala;
	bool trocou = false;
	int auxIdOFerta = -1;
	int auxCH = -1;
	pos1Hora = -1;
	pos2Hora = -1;
	pos1Dia = -1;
	pos2Dia = -1;
	pos1Sala = -1;
	pos2Sala = -1;

	do
	{
		/*
		pos1Hora = random(NUM_HORARIO); // posicao que ira apontar um horário de aula
		pos2Hora = random(NUM_HORARIO); // posicao que irá apontar outro horario de aula
		pos1Dia = random(NUM_DIA);
		pos2Dia = random(NUM_DIA);
		pos1Sala = random(NUM_SALA);
		pos2Sala = random(NUM_SALA);
		*/
		pos1Hora = rand() % (NUM_HORARIO); // posicao que ira apontar um horário de aula
		pos2Hora = rand() % (NUM_HORARIO); // posicao que irá apontar outro horario de aula
		pos1Dia = rand() % (NUM_DIA);
		pos2Dia = rand() % (NUM_DIA);
		pos1Sala = rand() % (NUM_SALA);
		pos2Sala = rand() % (NUM_SALA);

		if ((solucaoVizinho->gradeSala[pos1Sala].gradeHorario[pos1Hora][pos1Dia] >= 0 && solucaoVizinho->gradeSala[pos1Sala].gradeHorario[pos1Hora][pos1Dia] < NUM_OFERTA && solucaoVizinho->gradeSala[pos2Sala].gradeHorario[pos2Hora][pos2Dia] >= 0 && solucaoVizinho->gradeSala[pos2Sala].gradeHorario[pos2Hora][pos2Dia] < NUM_OFERTA) && (ofertas[solucaoVizinho->gradeSala[pos1Sala].gradeHorario[pos1Hora][pos1Dia]].ch == ofertas[solucaoVizinho->gradeSala[pos2Sala].gradeHorario[pos2Hora][pos2Dia]].ch))
		{
			//ajuste da posição do horário1 caso a posição inicial do swap tenha pegado a segunda posição de uma oferta alocada
			if (ofertas[solucaoVizinho->gradeSala[pos1Sala].gradeHorario[pos1Hora][pos1Dia]].ch > 1)
			{
				if (solucaoVizinho->gradeSala[pos1Sala].gradeHorario[pos1Hora][pos1Dia] == solucaoVizinho->gradeSala[pos1Sala].gradeHorario[pos1Hora - 1][pos1Dia])
				{
					if (solucaoVizinho->gradeSala[pos1Sala].gradeHorario[pos1Hora - 1][pos1Dia] == solucaoVizinho->gradeSala[pos1Sala].gradeHorario[pos1Hora - 2][pos1Dia])
					{
						if (solucaoVizinho->gradeSala[pos1Sala].gradeHorario[pos1Hora - 2][pos1Dia] == solucaoVizinho->gradeSala[pos1Sala].gradeHorario[pos1Hora - 3][pos1Dia])
						{
							pos1Hora -= 1;
						}
						pos1Hora -= 1;
					}
					pos1Hora -= 1;
				}
			}
			//ajuste da posição do horário2 caso a posição inicial do swap tenha pegado a segunda posição de uma oferta alocada
			if (ofertas[solucaoVizinho->gradeSala[pos2Sala].gradeHorario[pos2Hora][pos2Dia]].ch > 1)
			{
				if (solucaoVizinho->gradeSala[pos2Sala].gradeHorario[pos2Hora][pos2Dia] == solucaoVizinho->gradeSala[pos2Sala].gradeHorario[pos2Hora - 1][pos2Dia])
				{
					if (solucaoVizinho->gradeSala[pos2Sala].gradeHorario[pos2Hora - 1][pos2Dia] == solucaoVizinho->gradeSala[pos2Sala].gradeHorario[pos2Hora - 2][pos2Dia])
					{
						if (solucaoVizinho->gradeSala[pos2Sala].gradeHorario[pos2Hora - 2][pos2Dia] == solucaoVizinho->gradeSala[pos2Sala].gradeHorario[pos2Hora - 3][pos2Dia])
						{
							pos2Hora -= 1;
						}
						pos2Hora -= 1;
					}
					pos2Hora -= 1;
				}
			}
			if ((ofertas[solucaoVizinho->gradeSala[pos1Sala].gradeHorario[pos1Hora][pos1Dia]].turno == 2 && pos2Hora >= 9) || (ofertas[solucaoVizinho->gradeSala[pos1Sala].gradeHorario[pos1Hora][pos1Dia]].turno == 1 && pos2Hora < 9))
			{
				auxCH = ofertas[solucaoVizinho->gradeSala[pos1Sala].gradeHorario[pos1Hora][pos1Dia]].ch;
				for (int p = 0; p < auxCH; p++)
				{
					//if (solucaoVizinho->gradeSala[pos1Sala].gradeHorario[pos1Hora][pos1Dia] == 68){
					// printf("");
					//}
					auxIdOFerta = solucaoVizinho->gradeSala[pos2Sala].gradeHorario[pos2Hora + p][pos2Dia];
					solucaoVizinho->gradeSala[pos2Sala].gradeHorario[pos2Hora + p][pos2Dia] = solucaoVizinho->gradeSala[pos1Sala].gradeHorario[pos1Hora + p][pos1Dia];
					solucaoVizinho->gradeSala[pos1Sala].gradeHorario[pos1Hora + p][pos1Dia] = auxIdOFerta;
				}
				trocou = true;
			}
		}
	} while ((solucaoVizinho->gradeSala[pos1Sala].gradeHorario[pos1Hora][pos1Dia] <= -1 || solucaoVizinho->gradeSala[pos2Sala].gradeHorario[pos2Hora][pos2Dia] <= -1) && (trocou == false));
}

void geraLectureMove(Solucao *solucaoVizinho) {
	//random entre as posições que possuem aula
	int pos1Hora, pos2Hora, pos1Dia, pos2Dia, pos1Sala, pos2Sala;
	bool trocou = false;
	int auxIdOFerta = -1;
	int auxCH = -1;
	pos1Hora = -1;
	pos2Hora = -1;
	pos1Dia = -1;
	pos2Dia = -1;
	pos1Sala = -1;
	pos2Sala = -1;
	
	pos1Hora = rand() % (NUM_HORARIO); // posicao que ira apontar um horário de aula
	pos2Hora = rand() % (NUM_HORARIO); // posicao que irá apontar outro horario de aula
	pos1Dia = rand() % (NUM_DIA);
	pos2Dia = rand() % (NUM_DIA);
	pos1Sala = rand() % (NUM_SALA);
	pos2Sala = rand() % (NUM_SALA);
	if ((solucaoVizinho->gradeSala[pos1Sala].gradeHorario[pos1Hora][pos1Dia] >= 0 && solucaoVizinho->gradeSala[pos1Sala].gradeHorario[pos1Hora][pos1Dia] < NUM_OFERTA && solucaoVizinho->gradeSala[pos2Sala].gradeHorario[pos2Hora][pos2Dia] >= 0 && solucaoVizinho->gradeSala[pos2Sala].gradeHorario[pos2Hora][pos2Dia] < NUM_OFERTA) && (ofertas[solucaoVizinho->gradeSala[pos1Sala].gradeHorario[pos1Hora][pos1Dia]].ch == ofertas[solucaoVizinho->gradeSala[pos2Sala].gradeHorario[pos2Hora][pos2Dia]].ch))
	{
		//ajuste da posição do horário1 caso a posição inicial do swap tenha pegado a segunda posição de uma oferta alocada
		if (ofertas[solucaoVizinho->gradeSala[pos1Sala].gradeHorario[pos1Hora][pos1Dia]].ch > 1)
		{
			if (solucaoVizinho->gradeSala[pos1Sala].gradeHorario[pos1Hora][pos1Dia] == solucaoVizinho->gradeSala[pos1Sala].gradeHorario[pos1Hora - 1][pos1Dia])
			{
				if (solucaoVizinho->gradeSala[pos1Sala].gradeHorario[pos1Hora - 1][pos1Dia] == solucaoVizinho->gradeSala[pos1Sala].gradeHorario[pos1Hora - 2][pos1Dia])
				{
					if (solucaoVizinho->gradeSala[pos1Sala].gradeHorario[pos1Hora - 2][pos1Dia] == solucaoVizinho->gradeSala[pos1Sala].gradeHorario[pos1Hora - 3][pos1Dia])
					{
						pos1Hora -= 1;
					}
					pos1Hora -= 1;
				}
				pos1Hora -= 1;
			}
		}
		//ajuste da posição do horário2 caso a posição inicial do swap tenha pegado a segunda posição de uma oferta alocada
		if (ofertas[solucaoVizinho->gradeSala[pos2Sala].gradeHorario[pos2Hora][pos2Dia]].ch > 1)
		{
			if (solucaoVizinho->gradeSala[pos2Sala].gradeHorario[pos2Hora][pos2Dia] == solucaoVizinho->gradeSala[pos2Sala].gradeHorario[pos2Hora - 1][pos2Dia])
			{
				if (solucaoVizinho->gradeSala[pos2Sala].gradeHorario[pos2Hora - 1][pos2Dia] == solucaoVizinho->gradeSala[pos2Sala].gradeHorario[pos2Hora - 2][pos2Dia])
				{
					if (solucaoVizinho->gradeSala[pos2Sala].gradeHorario[pos2Hora - 2][pos2Dia] == solucaoVizinho->gradeSala[pos2Sala].gradeHorario[pos2Hora - 3][pos2Dia])
					{
						pos2Hora -= 1;
					}
					pos2Hora -= 1;
				}
				pos2Hora -= 1;
			}
		}
		if ((ofertas[solucaoVizinho->gradeSala[pos1Sala].gradeHorario[pos1Hora][pos1Dia]].turno == 2 && pos2Hora >= 9) || (ofertas[solucaoVizinho->gradeSala[pos1Sala].gradeHorario[pos1Hora][pos1Dia]].turno == 1 && pos2Hora < 9))
		{
			auxCH = ofertas[solucaoVizinho->gradeSala[pos1Sala].gradeHorario[pos1Hora][pos1Dia]].ch;
			for (int p = 0; p < auxCH; p++)
			{
				//if (solucaoVizinho->gradeSala[pos1Sala].gradeHorario[pos1Hora][pos1Dia] == 68){
				// printf("");
				//}
				auxIdOFerta = solucaoVizinho->gradeSala[pos2Sala].gradeHorario[pos2Hora + p][pos2Dia];
				solucaoVizinho->gradeSala[pos2Sala].gradeHorario[pos2Hora + p][pos2Dia] = solucaoVizinho->gradeSala[pos1Sala].gradeHorario[pos1Hora + p][pos1Dia];
				solucaoVizinho->gradeSala[pos1Sala].gradeHorario[pos1Hora + p][pos1Dia] = auxIdOFerta;
			}
			trocou = true;
		}
	}else if (solucaoVizinho->gradeSala[pos1Sala].gradeHorario[pos1Hora][pos1Dia] >= 0 && solucaoVizinho->gradeSala[pos2Sala].gradeHorario[pos2Hora][pos2Dia] == -1 && solucaoVizinho->gradeSala[pos1Sala].gradeHorario[pos1Hora][pos1Dia] < NUM_OFERTA  && solucaoVizinho->gradeSala[pos2Sala].gradeHorario[pos2Hora][pos2Dia] < NUM_OFERTA)
	{

		//ajuste da posição do horário1 caso a posição inicial do swap tenha pegado a segunda posição de uma oferta alocada
		if (ofertas[solucaoVizinho->gradeSala[pos1Sala].gradeHorario[pos1Hora][pos1Dia]].ch > 1)
		{
			if (solucaoVizinho->gradeSala[pos1Sala].gradeHorario[pos1Hora][pos1Dia] == solucaoVizinho->gradeSala[pos1Sala].gradeHorario[pos1Hora - 1][pos1Dia])
			{
				if (solucaoVizinho->gradeSala[pos1Sala].gradeHorario[pos1Hora - 1][pos1Dia] == solucaoVizinho->gradeSala[pos1Sala].gradeHorario[pos1Hora - 2][pos1Dia])
				{
					if (solucaoVizinho->gradeSala[pos1Sala].gradeHorario[pos1Hora - 2][pos1Dia] == solucaoVizinho->gradeSala[pos1Sala].gradeHorario[pos1Hora - 3][pos1Dia])
					{
						pos1Hora -= 1;
					}
					pos1Hora -= 1;
				}
				pos1Hora -= 1;
			}
		}

		if ((ofertas[solucaoVizinho->gradeSala[pos1Sala].gradeHorario[pos1Hora][pos1Dia]].turno == 2 && pos2Hora >= 9) || (ofertas[solucaoVizinho->gradeSala[pos1Sala].gradeHorario[pos1Hora][pos1Dia]].turno == 1 && pos2Hora < 9))
		{

			if (verificaEspaco(solucaoVizinho, ofertas[solucaoVizinho->gradeSala[pos1Sala].gradeHorario[pos1Hora][pos1Dia]].ch, pos2Dia, pos2Hora, pos2Sala))
			{

				auxCH = ofertas[solucaoVizinho->gradeSala[pos1Sala].gradeHorario[pos1Hora][pos1Dia]].ch;
				for (int p = 0; p < auxCH; p++)
				{
					aux = solucaoVizinho->gradeSala[pos1Sala].gradeHorario[pos1Hora + p][pos1Dia];
					solucaoVizinho->gradeSala[pos2Sala].gradeHorario[pos2Hora + p][pos2Dia] = aux;
					solucaoVizinho->gradeSala[pos1Sala].gradeHorario[pos1Hora + p][pos1Dia] = -1;
				}

				trocou = true;
			}
		}
	}
}


void geraMove(Solucao *solucaoVizinho) {
	//random entre as posições que não possuem aula
	int pos1Hora, pos2Hora, pos1Dia, pos2Dia, pos1Sala, pos2Sala, aux;
	aux = -1;
	int auxCH = -1;
	bool trocou = false;
	pos1Hora = -1;
	pos2Hora = -1;
	pos1Dia = -1;
	pos2Dia = -1;
	pos1Sala = -1;
	pos2Sala = -1;

	//procura uma posição alocada e uma posição desalocada para mover uma oferta

	do
	{
		/*
		pos1Hora = random(NUM_HORARIO); // posicao que ira apontar um horário de aula
		pos2Hora = random(NUM_HORARIO); // posicao que irá apontar outro horario de aula
		pos1Dia = random(NUM_DIA);
		pos2Dia = random(NUM_DIA);
		pos1Sala = random(NUM_SALA);
		pos2Sala = random(NUM_SALA);
		*/
		pos1Hora = rand() % (NUM_HORARIO); // posicao que ira apontar um horário de aula
		pos2Hora = rand() % (NUM_HORARIO); // posicao que irá apontar outro horario de aula
		pos1Dia = rand() % (NUM_DIA);
		pos2Dia = rand() % (NUM_DIA);
		pos1Sala = rand() % (NUM_SALA);
		pos2Sala = rand() % (NUM_SALA);

		if (solucaoVizinho->gradeSala[pos1Sala].gradeHorario[pos1Hora][pos1Dia] >= 0 && solucaoVizinho->gradeSala[pos2Sala].gradeHorario[pos2Hora][pos2Dia] == -1 && solucaoVizinho->gradeSala[pos1Sala].gradeHorario[pos1Hora][pos1Dia] < NUM_OFERTA  && solucaoVizinho->gradeSala[pos2Sala].gradeHorario[pos2Hora][pos2Dia] < NUM_OFERTA)
		{

			//ajuste da posição do horário1 caso a posição inicial do swap tenha pegado a segunda posição de uma oferta alocada
			if (ofertas[solucaoVizinho->gradeSala[pos1Sala].gradeHorario[pos1Hora][pos1Dia]].ch > 1)
			{
				if (solucaoVizinho->gradeSala[pos1Sala].gradeHorario[pos1Hora][pos1Dia] == solucaoVizinho->gradeSala[pos1Sala].gradeHorario[pos1Hora - 1][pos1Dia])
				{
					if (solucaoVizinho->gradeSala[pos1Sala].gradeHorario[pos1Hora - 1][pos1Dia] == solucaoVizinho->gradeSala[pos1Sala].gradeHorario[pos1Hora - 2][pos1Dia])
					{
						if (solucaoVizinho->gradeSala[pos1Sala].gradeHorario[pos1Hora - 2][pos1Dia] == solucaoVizinho->gradeSala[pos1Sala].gradeHorario[pos1Hora - 3][pos1Dia])
						{
							pos1Hora -= 1;
						}
						pos1Hora -= 1;
					}
					pos1Hora -= 1;
				}
			}

			if ((ofertas[solucaoVizinho->gradeSala[pos1Sala].gradeHorario[pos1Hora][pos1Dia]].turno == 2 && pos2Hora >= 9) || (ofertas[solucaoVizinho->gradeSala[pos1Sala].gradeHorario[pos1Hora][pos1Dia]].turno == 1 && pos2Hora < 9))
			{

				if (verificaEspaco(solucaoVizinho, ofertas[solucaoVizinho->gradeSala[pos1Sala].gradeHorario[pos1Hora][pos1Dia]].ch, pos2Dia, pos2Hora, pos2Sala))
				{

					auxCH = ofertas[solucaoVizinho->gradeSala[pos1Sala].gradeHorario[pos1Hora][pos1Dia]].ch;
					for (int p = 0; p < auxCH; p++)
					{
						aux = solucaoVizinho->gradeSala[pos1Sala].gradeHorario[pos1Hora + p][pos1Dia];
						solucaoVizinho->gradeSala[pos2Sala].gradeHorario[pos2Hora + p][pos2Dia] = aux;
						solucaoVizinho->gradeSala[pos1Sala].gradeHorario[pos1Hora + p][pos1Dia] = -1;
					}

					trocou = true;
				}
			}
		}
	} while ((solucaoVizinho->gradeSala[pos1Sala].gradeHorario[pos1Hora][pos1Dia] <= -1 || solucaoVizinho->gradeSala[pos2Sala].gradeHorario[pos2Hora][pos2Dia] != -1) && (trocou == false));
}



void geraSimpleKempe(Solucao *solucaoVizinho) {
	//random entre as posições que possuem aula
	int pos1Hora, pos2Hora, pos1Dia, pos2Dia, pos1Sala, pos2Sala;
	int matrizConflito[14][14];
	int aux = -1;
	int aux2 = -1;
	bool trocou = false;
	int auxIdOFerta = -1;
	int auxCH = -1;
	pos1Hora = -1;
	pos2Hora = -1;
	pos1Dia = -1;
	pos2Dia = -1;
	pos1Sala = -1;
	pos2Sala = -1;
	int t1[14];
	int t2[14];

	do {
		pos1Dia = rand() % (NUM_DIA);
		pos2Dia = rand() % (NUM_DIA);
		pos1Sala = rand() % (NUM_SALA);
		pos2Sala = rand() % (NUM_SALA);
		pos1Hora = 0;
		pos2Hora = 0;

		//inicia matrizConflito 
		for (int i = 0; i < NUM_HORARIO; i++) {
			for (int j = 0; j < NUM_HORARIO; j++) {
				matrizConflito[i][j] = -1;
			}
		}
		
		//preenche vetor t1
		for (int i = 0; i < NUM_HORARIO; i++) {
			t1[i] = solucaoVizinho->gradeSala[pos1Sala].gradeHorario[i][pos1Dia];
		}

		//preenche vetor t2
		for (int j = 0; j < NUM_HORARIO; j++) {
			t2[j] = solucaoVizinho->gradeSala[pos2Sala].gradeHorario[j][pos2Dia];
		}

		
		for (int i = 0; i < NUM_HORARIO; i++) {
			for (int j = 0; j < NUM_HORARIO; j++) {
				
				//preenche matriz de conflito
				if ((ofertas[solucaoVizinho->gradeSala[pos1Sala].gradeHorario[i][pos1Dia]].idProfessor == ofertas[solucaoVizinho->gradeSala[pos2Sala].gradeHorario[j][pos2Dia]].idProfessor) || (ofertas[solucaoVizinho->gradeSala[pos1Sala].gradeHorario[i][pos1Dia]].turmas == ofertas[solucaoVizinho->gradeSala[pos2Sala].gradeHorario[j][pos2Dia]].turmas)) {
					matrizConflito[i][j] = 1;
				}
			}
		}

		//troca das disciplinas de acordo com a matriz de conflito
		for (int j = 0; j < NUM_HORARIO; j++) {
			for (int i = 0; i < NUM_HORARIO; i++) {
				aux = -1;
				aux2 = -1;
				pos1Hora = i;
				pos2Hora = j;
				
				if ((matrizConflito[i][j]  == 1)  && (ofertas[solucaoVizinho->gradeSala[pos1Sala].gradeHorario[i][pos1Dia]].ch == ofertas[solucaoVizinho->gradeSala[pos2Sala].gradeHorario[j][pos2Dia]].ch)) {
					
					if ((ofertas[solucaoVizinho->gradeSala[pos1Sala].gradeHorario[pos1Hora][pos1Dia]].turno == 2 && pos2Hora >= 9) || (ofertas[solucaoVizinho->gradeSala[pos1Sala].gradeHorario[pos1Hora][pos1Dia]].turno == 1 && pos2Hora < 9)){
						
						auxCH = ofertas[solucaoVizinho->gradeSala[pos1Sala].gradeHorario[pos1Hora][pos1Dia]].ch;

						if (ofertas[solucaoVizinho->gradeSala[pos1Sala].gradeHorario[pos1Hora][pos1Dia]].turno == 2) {
							if ((pos1Hora + auxCH <= 13) && (pos2Hora + auxCH <= 13)) {
								for (int p = 0; p < auxCH; p++) {
									aux = solucaoVizinho->gradeSala[pos1Sala].gradeHorario[i][pos1Dia];
									solucaoVizinho->gradeSala[pos1Sala].gradeHorario[i][pos1Dia] = solucaoVizinho->gradeSala[pos2Sala].gradeHorario[j][pos2Dia];
									solucaoVizinho->gradeSala[pos2Sala].gradeHorario[j][pos2Dia] = aux;
									matrizConflito[i + p][j + p] = -1;
								}
								i = i + auxCH;
								trocou = true;
							}
						}
						else {
							if ((pos1Hora + auxCH < 9) && (pos2Hora + auxCH < 9)) {
								for (int p = 0; p < auxCH; p++) {
									aux = solucaoVizinho->gradeSala[pos1Sala].gradeHorario[i][pos1Dia];
									solucaoVizinho->gradeSala[pos1Sala].gradeHorario[i][pos1Dia] = solucaoVizinho->gradeSala[pos2Sala].gradeHorario[j][pos2Dia];
									solucaoVizinho->gradeSala[pos2Sala].gradeHorario[j][pos2Dia] = aux;
									matrizConflito[i + p][j + p] = -1;
								}
								i = i + auxCH;
								trocou = true;
							}
						}
						/*
						if ((auxCH == 2) && ((pos1Hora + 2) <= 13)) {
							//if (solucaoVizinho->gradeSala[pos1Sala].gradeHorario[i + 1][pos1Dia] == solucaoVizinho->gradeSala[pos1Sala].gradeHorario[i][pos1Dia]) {
								aux = solucaoVizinho->gradeSala[pos1Sala].gradeHorario[i][pos1Dia];
								solucaoVizinho->gradeSala[pos1Sala].gradeHorario[i][pos1Dia] = solucaoVizinho->gradeSala[pos2Sala].gradeHorario[j][pos2Dia];
								solucaoVizinho->gradeSala[pos2Sala].gradeHorario[j][pos2Dia] = aux;

								aux2 = solucaoVizinho->gradeSala[pos1Sala].gradeHorario[i + 1][pos1Dia];
								solucaoVizinho->gradeSala[pos1Sala].gradeHorario[i + 1][pos1Dia] = solucaoVizinho->gradeSala[pos2Sala].gradeHorario[j + 1][pos2Dia];
								solucaoVizinho->gradeSala[pos2Sala].gradeHorario[j + 1][pos2Dia] = aux2;
							//}
						}
							else {
								aux = solucaoVizinho->gradeSala[pos1Sala].gradeHorario[i][pos1Dia];
								solucaoVizinho->gradeSala[pos1Sala].gradeHorario[i][pos1Dia] = solucaoVizinho->gradeSala[pos2Sala].gradeHorario[j][pos2Dia];
								solucaoVizinho->gradeSala[pos2Sala].gradeHorario[j][pos2Dia] = aux;
							}
						
						//atualiza i para pular para a próxima aula que seja de outra disciplina
						i = i + auxCH - 1;
						//j = j + auxCH - 1;
						trocou = true;
						*/
					}
					
					

					/*
					//ajuste da posição do horário1 caso a posição inicial do swap tenha pegado a segunda posição de uma oferta alocada
					if (ofertas[solucaoVizinho->gradeSala[pos1Sala].gradeHorario[pos1Hora][pos1Dia]].ch > 1)
					{
						if (solucaoVizinho->gradeSala[pos1Sala].gradeHorario[pos1Hora][pos1Dia] == solucaoVizinho->gradeSala[pos1Sala].gradeHorario[pos1Hora - 1][pos1Dia])
						{
							if (solucaoVizinho->gradeSala[pos1Sala].gradeHorario[pos1Hora - 1][pos1Dia] == solucaoVizinho->gradeSala[pos1Sala].gradeHorario[pos1Hora - 2][pos1Dia])
							{
								if (solucaoVizinho->gradeSala[pos1Sala].gradeHorario[pos1Hora - 2][pos1Dia] == solucaoVizinho->gradeSala[pos1Sala].gradeHorario[pos1Hora - 3][pos1Dia])
								{
									pos1Hora -= 1;
								}
								pos1Hora -= 1;
							}
							pos1Hora -= 1;
						}
					}
					//ajuste da posição do horário2 caso a posição inicial do swap tenha pegado a segunda posição de uma oferta alocada
					if (ofertas[solucaoVizinho->gradeSala[pos2Sala].gradeHorario[pos2Hora][pos2Dia]].ch > 1)
					{
						if (solucaoVizinho->gradeSala[pos2Sala].gradeHorario[pos2Hora][pos2Dia] == solucaoVizinho->gradeSala[pos2Sala].gradeHorario[pos2Hora - 1][pos2Dia])
						{
							if (solucaoVizinho->gradeSala[pos2Sala].gradeHorario[pos2Hora - 1][pos2Dia] == solucaoVizinho->gradeSala[pos2Sala].gradeHorario[pos2Hora - 2][pos2Dia])
							{
								if (solucaoVizinho->gradeSala[pos2Sala].gradeHorario[pos2Hora - 2][pos2Dia] == solucaoVizinho->gradeSala[pos2Sala].gradeHorario[pos2Hora - 3][pos2Dia])
								{
									pos2Hora -= 1;
								}
								pos2Hora -= 1;
							}
							pos2Hora -= 1;
						}
					}
					*/
					/*
					if ((ofertas[solucaoVizinho->gradeSala[pos1Sala].gradeHorario[pos1Hora][pos1Dia]].turno == 2 && pos2Hora >= 9) || (ofertas[solucaoVizinho->gradeSala[pos1Sala].gradeHorario[pos1Hora][pos1Dia]].turno == 1 && pos2Hora < 9)) {
							auxCH = ofertas[solucaoVizinho->gradeSala[pos1Sala].gradeHorario[pos1Hora][pos1Dia]].ch;
							for (int p = 0; p < auxCH; p++)
							{
								aux = solucaoVizinho->gradeSala[pos1Sala].gradeHorario[pos1Hora + p][pos1Dia];
								solucaoVizinho->gradeSala[pos1Sala].gradeHorario[pos1Hora + p][pos1Dia] = solucaoVizinho->gradeSala[pos2Sala].gradeHorario[pos2Hora + p][pos2Dia];
								solucaoVizinho->gradeSala[pos2Sala].gradeHorario[pos2Hora + p][pos2Dia] = aux;
								matrizConflito[i][j] = -1;
							}
							//atualiza i para pular para a próxima aula que seja de outra disciplina
							i = i + auxCH - 1;
							trocou = true;
					}
					*/
				}
			}
		}
		//trocou = true;
	} while (trocou == false);
}

//-------------------------Simulated Annealing------------------------
void simulatedAnnealing() {

	Solucao *solucaoInicial, *solucaoAtual, *solucaoVizinho, *melhorSolucao;
	double tempInicial = 0;
	double tempFinal = 0;
	double tempIteracao = 0;
	double taxaResfr = 0;
	double probVizinho = 0;
	double delta = 0;
	double x = 0;
	double valorExp = 0;
	double tempExe = 0;
	double tempMelhorSol = 0;
	int numIteracoes = 0;
	int numIterMax = 0;

	taxaResfr = TAXA_RESFRIAMENTO;
	tempInicial = TEMP_INICIAL;
	tempIteracao = TEMP_INICIAL;
	tempFinal = TEMP_FINAL;
	numIterMax = NUM_MAX_ITERACOES;

	solucaoInicial = &vetSol[0];
	solucaoAtual = &vetSol[1];
	solucaoVizinho = &vetSol[2];
	melhorSolucao = &vetSol[3];
	calcFO(solucaoInicial);
	clonarSolucao(solucaoInicial, solucaoAtual);
	calcFO(solucaoAtual);
	clonarSolucao(solucaoInicial, melhorSolucao);
	calcFO(melhorSolucao);

	//iH = clock();
	//while((tempExe - tempMelhorSol) < TEMPO_LIMITE)
	//{
	numIteracoes = 0;
	tempIteracao = tempInicial;
	while (tempIteracao > tempFinal)
	{
		while (numIteracoes < numIterMax)
		{
			numIteracoes++;
			clonarSolucao(solucaoAtual, solucaoVizinho);

			probVizinho = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
			if (probVizinho < PROB_MOVIMENTO_KEMPE)
			{
				geraSimpleKempe(solucaoVizinho);
			}
			else if (probVizinho >= PROB_MOVIMENTO_KEMPE && probVizinho < PROB_MOVIMENTO_SWAP) {
				geraSwap(solucaoVizinho);
			}
			else {
				geraMove(solucaoVizinho);
			}
			
			//geraLectureMove(solucaoVizinho);
			
			calcFO(solucaoVizinho);
			delta = solucaoVizinho->fo - solucaoAtual->fo;

			if (delta < 0)
			{
				clonarSolucao(solucaoVizinho, solucaoAtual);
				if (solucaoAtual->fo < melhorSolucao->fo)
				{

					clonarSolucao(solucaoAtual, melhorSolucao);
					printf("FO da melhor solucao SA: %d\n", melhorSolucao->fo);
				}
			}
			else
			{
				x = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
				valorExp = pow(M_E, -delta / tempIteracao);
				if (x < valorExp)
				{
					clonarSolucao(solucaoVizinho, solucaoAtual);
				}
			}
		}
		tempIteracao = tempIteracao * taxaResfr;
		numIteracoes = 0;
		fH = clock();
		if ((((double)fH - (double)iH) / CLOCKS_PER_SEC) >= MAX_TEMPO)
		{
			break;
		}
	}

	//tempExe = ((double)fH - (double)iH)/CLOCKS_PER_SEC;
	//tempIteracao = tempInicial;
	//}
}
//------------------------------FIM SA---------------------------

//Lê solução -----------------------------------------------------------------
void lerSolucao(char caminho[], Solucao *sol, int sala) {
	FILE * arqSolucao = fopen(caminho, "r");

	if (arqSolucao != NULL) {
		for (int h = 0; h < NUM_HORARIO; h++)
		{
			fscanf(arqSolucao, "%d", &(sol->gradeSala[sala].gradeHorario[h][0])
			);
			fscanf(arqSolucao, "%d", &(sol->gradeSala[sala].gradeHorario[h][1])
			);
			fscanf(arqSolucao, "%d", &(sol->gradeSala[sala].gradeHorario[h][2])
			);
			fscanf(arqSolucao, "%d", &(sol->gradeSala[sala].gradeHorario[h][3])
			);
			fscanf(arqSolucao, "%d", &(sol->gradeSala[sala].gradeHorario[h][4])
			);
		}
	}
	fclose(arqSolucao);
}



//-----------------------------------------------------------------------------


//----------------------------------------------------------------------
void lerArquivos() {
	lerSalas();
	lerTipoSalas();
	lerHorarios();
	lerProfessores();
	lerTurmas();
	lerOfertas();
	lerDisciplinas();
}

void imprimirPenalizacoes() {

	printf("W1: %d \n", CONF_PROFESSOR);
	printf("W2: %d \n", CONF_TURMA);
	printf("W3: %d \n", CONF_SALA);
	printf("W4: %d \n", CONF_TURMA_CAPACIDADE);
	printf("W5: %d \n", CONF_TP_INC_SALA);
	printf("W6: %d \n", CONF_AULA_3H);
	printf("W7: %d \n", CONF_FORA_TURNO_OFRT);
	printf("W8: %d \n", CONF_INT_AULA_PROF);
	printf("W9: %d \n", CONF_INT_HOR_TURMA);
	printf("W10: %d \n", CONF_FORA_PER_PREF);
	printf("W11: %d \n", CONF_AULAS_SEG_DISC);
	printf("W12: %d \n", CONF_PROF_NOITE_MANHA);
	printf("W13: %d \n", CONF_AULAS_SEG_DIF);
	printf("W14: %d \n", CONF_AULA_DIF_ULT_HOR);
	printf("W15: %d \n", CONF_AULA_PAR);
	printf("W16: %d \n", CONF_AULA_ALMOCO);
	printf("W17: %d \n", CONF_FORA_HORA_PADRAO);
}

void exportarMelhorSolucao() {
	exportaSala(&vetSol[4], 1);
	exportaTurma(&vetSol[4], 1);
	exportaProfessor(&vetSol[4], 1);
}

void exportarSolucaoInicial() {
	exportaSala(&vetSol[0], 1);
	exportaTurma(&vetSol[0], 1);
	exportaProfessor(&vetSol[0], 1);
}

void executarGrasp() {
	double tempMelhorSol;
	iniciarSolucao(&vetSol[4]);
	iH = clock();
	while ((((double)fH - (double)iH) / CLOCKS_PER_SEC) <= MAX_TEMPO) {
		printf("Executando Grasp \n");
		iniciarSolucao(&vetSol[0]);
		iniciarSolucao(&vetSol[1]);
		iniciarSolucao(&vetSol[2]);
		iniciarSolucao(&vetSol[3]);
		criarSolucaoInicial(&vetSol[0]);
		//exportarSolucaoInicial();
		simulatedAnnealing();
		if (vetSol[3].fo < vetSol[4].fo)
		{
			bstH = clock();
			tempMelhorSol = ((double)bstH - (double)iH) / CLOCKS_PER_SEC;
			clonarSolucao(&vetSol[3], &vetSol[4]);
			printf("Melhor solucao GRASP FO: %d,Tempo: %f\n", vetSol[4].fo, tempMelhorSol);
		}

		fH = clock();
	}

	exportaParametros(&vetSol[4], tempMelhorSol);

}

int main() {
	lerArquivos();
	iniciarSolucao(&vetSol[0]);
	iniciarSolucao(&vetSol[1]);
	iniciarSolucao(&vetSol[2]);
	iniciarSolucao(&vetSol[3]);

#ifdef SOLUCAOCCA2016
	//----Lendo a solução final para conferência de resultados e penalidades------
	lerSolucao("solucao/sCCA2016/0.txt", &vetSol[3], 0);
	lerSolucao("solucao/sCCA2016/1.txt", &vetSol[3], 1);
	lerSolucao("solucao/sCCA2016/2.txt", &vetSol[3], 2);
	lerSolucao("solucao/sCCA2016/3.txt", &vetSol[3], 3);
	lerSolucao("solucao/sCCA2016/4.txt", &vetSol[3], 4);
	lerSolucao("solucao/sCCA2016/5.txt", &vetSol[3], 5);
	lerSolucao("solucao/sCCA2016/6.txt", &vetSol[3], 6);
	lerSolucao("solucao/sCCA2016/7.txt", &vetSol[3], 7);
	lerSolucao("solucao/sCCA2016/8.txt", &vetSol[3], 8);
	lerSolucao("solucao/sCCA2016/9.txt", &vetSol[3], 9);
	calcFO(&vetSol[3]);
	printf("%ld", vetSol[3].fo);
	imprimirPenalizacoes();
# else

	executarGrasp();
	exportarMelhorSolucao();

#endif

}