#include <QtTest>
#include "microtubule.h"

/* Unit tests for the grow function of the Microtubule class. The tests verifies
 * that the basic functionality of the function works as expected.
 *
 * The logic is: construct an MT with a known length of 10.0, call grow(3.5), 
 * and assert the result is 13.5.
 */

class TestMicrotubule : public QObject
{
  Q_OBJECT

private slots:
  void grow_increasesLength();
};

void TestMicrotubule::grow_increasesLength()
{
  Microtubule mt(0, 0, 10.0);
  mt.grow(3.5);
  QCOMPARE(mt.getLength(), 13.5);
}

QTEST_APPLESS_MAIN(TestMicrotubule)
#include "test_microtubule.moc"