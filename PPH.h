#define NUM_PROFESSOR 24   //total de professores
#define NUM_SALA 10	      //total de salas disponíveis
#define NUM_TIPO_SALA 2    //total de tipos de salas
#define NUM_HORARIO 14     //horarios disponiveis da sala
#define NUM_TURMA 14      //quantidade de turmas
#define NUM_OFERTA 80      //quantidade de ofertas de disciplinas
#define NUM_BLOQ 13        //quantidade de ofertas de bloqueadas
#define NUM_DISCIPLINA 72  //quantidade de disciplinas
#define NUM_DIA 5 		   //total de dias da semana
#define NUM_MAX_CANDIDATOS 700 //é o valor de combinações possíveis de dias X horas X salas
#define ALFA 0.15          //valor alfa utilizado para fazer o corte na lista de candidatos
#define MAX_SOLUCAO 5      // número de soluções que podem ser aramzenadas no vetSol
#define TAXA_RESFRIAMENTO 0.975 //Fator de redução da temperatura
#define TEMP_INICIAL 1000
#define TEMP_FINAL 0.01
#define NUM_MAX_ITERACOES 500
#define TEMPO_LIMITE  90 //tempo limite de execução do método sa
#define PROB_MOVIMENTO_SWAP 0.55  //probabilidade de swap 
#define PROB_MOVIMENTO_KEMPE 0.1  //probabilidade simple kempe
#define MAX_TEMPO 500.0 //Tempo máximo para execução do GRASP

typedef struct tPosicao{
	int sala;
	int dia;
	int horaIni;
}Posicao;

typedef struct tProblema {
	int quantSalas;
	int quantHorarios;
	int quantProfessores;
	int quantTurmas;
	int quantOfertas;
	int quantDisciplinas;
	int quantTipoSalas;

}Problema;

typedef struct tSala {
	int id;
	char predio[100];
	int idTipoSala;
	int numSala;
	int capacidade;
}Sala;

typedef struct tTipoSala{
	int id;
	char nome[70];
}TipoSala;

typedef struct tHorario {
	int id;
	char inicio[6];
	char fim[6];
	int duracao;
}Horario;

typedef struct tProfessor {
	int id;
	char nome[100];
}Professor;

typedef struct tTurma {
	int id;
	char nome[100];
	int periodo;
	int turnoPreferencial; // 0 - matutino, 1- vespertino, 2 - noturno
}Turma;

typedef struct tOferta {
	int id;
	int idDisciplina;
	int numTurmas; // numero de turmas que a oferta esta disponivel
	int *turmas;	// turmas que a oferta esta disponivel como obrigatoria (curso e período, podendo ser por exemplo: CC 1º e EQ 1º)
	int numVagas;  	//vagas ofertadas
	int turno; //1- diuno , 2-noturno
	int idProfessor;
	int idTipoSala;
	int ch; 		//carga horaria da oferta: 1,2,3,4 ou 5h
}Oferta;

typedef struct tDisciplina {
	int id;
	char codigo[10];
	char nome[100];
	int nivel; // 0- normal, 1- dificil
}Disciplina;


typedef struct tGradeHorario {
	int gradeHorario[NUM_HORARIO][NUM_DIA];
}GradeHorario;

typedef struct tSolucao
{
	GradeHorario gradeSala[NUM_SALA];
	GradeHorario gradeProfessor[NUM_PROFESSOR];
	GradeHorario gradeTurma[NUM_TURMA];
	long fo;
}Solucao;

typedef struct tListaOfertas{
	int idOferta;
	int numVagasOferta;
	int idTipoSalaOferta;
   int chOferta;
   int turnoOferta;
}ListaOfertas;

typedef struct tListaCandidatos{
	int idOferta;
   long fo;
   int sala;
   int dia ;
   int hora;
   int ch ;
}ListaCandidatos;

//****************************************************************************

//************************** Métodos *****************************************

void grasp();
void criarSolucaoInicial(Solucao * sl);
void criarSolucao(Solucao *sl);
bool verificaEspaco(Solucao * sol, int tam, int dia, int horario, int sala);
void ordenaNumVagas (ListaOfertas *listaOfertas);
void carregarHorarioBloqueadoSala(Solucao *sol);
void exportaSala(Solucao * ss, int posR);
void exportaTurma(Solucao * ss,int posR);
void exportaProfessor(Solucao * ss,int posR);
void clonarSolucao(Solucao *original, Solucao *clone);
void simulatedAnnealing(Solucao *s);
void geraSwap(Solucao *solucaoVizinho);
void geraLectureMove(Solucao *solucaoVizinho);
void geraMove(Solucao *solucaoVizinho);
void printMelhorSolucao();
void printSolucaoInicial();
void imprimirPenalizacoes();
void exportarSolucaoInicial();
void exportarMelhorSolucao();
void exportaParametros(Solucao *sol, double melhorTempo);
// ------------ Cálculo da FO
void calcFO(Solucao *sol);
void gerarTabelasPeT(Solucao * sol);
long confAula3h(Solucao *sol);
long confProfessor(Solucao *sol);
long confAula(Solucao *sol);
void iniciaProfessorTurma(Solucao * sl);
void iniciarVariaveis();
//****************************************************************************

//***************** Entrada de dados *****************************************
void lerArquivos();
void lerProfessores();
void lerHorarios();
void lerSalas() ;
void lerTipoSalas();
void lerTurmas();
void lerOfertas();
void lerDisciplinas() ;

