/*
 * MVRTreePlaygroung.cc
 *
 *  Created on: Feb 14, 2011
 *      Author: jarod
 */

#include <cstring>
#include <spatialindex/SpatialIndex.h>

using namespace spatialindex;
using namespace std;

#define DELETE 0
#define INSERT 1
#define QUERY 2

/**
 * A visitor class for processing the data
 * entry returned by the query.
 */
class PrintVisitor: public IVisitor {
public:
    void visitNode(const INode& n) {
    }

    /**
     * Print the data entry to the stdout.
     *
     * @param d
     */
    void visitData(const IData& d) {
        cout << d.getIdentifier() << endl;
    }

    void visitData(std::vector<const IData*>& v) {
    }
};

int main(int argc, char** argv) {

}
