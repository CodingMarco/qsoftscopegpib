#include "mainwindow.h"
#include <QApplication>


//typedef struct {
//	unsigned int rtop;
//	int scale;
//	// ...
//} rdiv_t;

//void calc_rtop(int rtop, rdiv_t *rdiv_negerr)
//{
//	// ...
//	rdiv_negerr->rtop = rtop;
//}

//int main()
//{
//	rdiv_t r1;
//	calc_rtop(33, &r1);
//	printf("%d\n", r1.rtop); // --> 33
//}






int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	MainWindow w;
	w.show();
	return a.exec();
}
