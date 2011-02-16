/*
 * MVRTreePlaygroung.cc
 *
 *  Created on: Feb 14, 2011
 *      Author: jarod
 */

#include <cstring>
#include <spatialindex/SpatialIndex.h>
#include <limits>

using namespace SpatialIndex;
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
    size_t m_index_io;
    size_t m_leaf_io;

    PrintVisitor(): m_index_io(0), m_leaf_io(0) {}

    void visitNode(const INode& n) {
        if(n.isLeaf())
            m_leaf_io++;
        else
            m_index_io++;
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

/**
 * A query strategy of find the total indexed space
 * managed by the index (the MBR of the root).
 */
class SpaceInspectStrategy: public IQueryStrategy {
public:
    Region m_indexedSpace;

public:
    void getNextEntry(const IEntry& entry, id_type& nextEntry, bool& hasNext) {
        // the first time we are called, entry points to the root.

        // stop after the root.
        hasNext = false;

        IShape* ps;
        entry.getShape(&ps);
        ps->getMBR(m_indexedSpace);
        delete ps;
    }
};

void getProperties(Tools::PropertySet& tree_properties, const char* config_file) {
    ifstream fin(config_file);
    if (!fin) {
        cerr << "Failed to load configure file: " << config_file << "." << endl;
    } else {
        string property_name, string_value;
        Tools::Variant value;
        while (fin) {
            fin >> property_name >> string_value;
            if (!fin.good())
                continue;
            if (!property_name.compare("Dimension") || !property_name.compare(
                    "IndexCapacity") || !property_name.compare("LeafCapacity")
                    || !property_name.compare("IndexPoolCapacity")
                    || !property_name.compare("LeafPoolCapacity")
                    || !property_name.compare("RegionPoolCapacity")
                    || !property_name.compare("PointPoolCapacity")
                    || !property_name.compare("NearMinimumOverlapFactor")) {
                value.m_varType = Tools::VT_ULONG;
                value.m_val.ulVal = atol(string_value.c_str());
            } else if (!property_name.compare("FillFactor")
                    || !property_name.compare("SplitDistributionFactor")
                    || !property_name.compare("ReinsertFactor")
                    || !property_name.compare("StrongVersionOverflow")
                    || !property_name.compare("VersionUnderflow")) {
                value.m_varType = Tools::VT_DOUBLE;
                value.m_val.dblVal = atof(string_value.c_str());
            } else {
                value.m_varType = Tools::VT_BOOL;
                value.m_val.blVal = (atoi(string_value.c_str()) == 0 ? false
                        : true);
            }
            tree_properties.setProperty(property_name, value);
        }
    }
}

int main(int argc, char** argv) {
    // Test arguments
    if (argc != 5 && argc != 3) {
        cerr << "Usage: \n" << argv[0]
                << " tree_file number_of_query number_of_data config_file\n"
                << "OR " << argv[0] << " tree_file number_of_query." << endl;
        return -1;
    }

    string index_name = argv[1];

    try {
        // Create storage manager with 4K page size
        IStorageManager* storage_manager =
                StorageManager::createNewDiskStorageManager(index_name, 4096);
        // Apply a random buffer on the storage manager with capacity of 32 pages
        StorageManager::IBuffer* buffer =
                StorageManager::createNewRandomEvictionsBuffer(
                        *storage_manager, 32, false);

        ISpatialIndex* tree;
        size_t time, count;
        Tools::Random rand;
        double plow[2], phigh[2];

        if (argc == 5) {
            // Need to generate original insertion data
            // Load the properties
            Tools::PropertySet tree_properties;
            getProperties(tree_properties, argv[4]);
            cout << "List Properties:\n " << tree_properties << endl;
            ifstream fin(argv[4]);
            if (!fin) {
                cerr << "Failed to load configure file: " << argv[4] << "."
                        << endl;
                return -1;
            } else {
                string property_name, string_value;
                Tools::Variant value;
                while (fin) {
                    fin >> property_name >> string_value;
                    if (!fin.good())
                        continue;
                }
            }
            // Create a new tree
            tree = MVRTree::returnMVRTree(*buffer, tree_properties);
            // Generate data
            size_t number_of_insertions = atoi(argv[3]);

            for (size_t i = 0; i < number_of_insertions; i++) {
                plow[0] = rand.nextUniformDouble();
                plow[1] = rand.nextUniformDouble();
                phigh[0] = plow[0] + rand.nextUniformDouble(0.0001, 0.1);
                phigh[1] = plow[1] + rand.nextUniformDouble(0.0001, 0.1);
                if (rand.nextUniformDouble() >= 0.8)
                    time++;
                TimeRegion temp_region = TimeRegion(plow, phigh, time, time, 2);
                tree->insertData(0, 0, temp_region, 1);
                count++;
            }

        } else {
            // Load tree from the data file
            tree = MVRTree::loadMVRTree(*buffer, 1);
        }

        PrintVisitor visitor;

        // Do queries
        // Generate query data
        for (int i = 0; i < atoi(argv[2]); i++) {
            plow[0] = rand.nextUniformDouble();
            plow[1] = rand.nextUniformDouble();
            phigh[0] = plow[0] + rand.nextUniformDouble(0.001, 0.2);
            phigh[1] = plow[1] + rand.nextUniformDouble(0.001, 0.2);
            TimeRegion temp_region = TimeRegion(plow, phigh,
                    rand.nextUniformUnsignedLong(0, time / 2),
                    rand.nextUniformUnsignedLong(time / 2 + 1, time), 2);

            // Do Query
            tree->intersectsWithQuery(temp_region, visitor);
            count++;
        }

        size_t index_io = visitor.m_index_io;
        size_t leaf_io = visitor.m_leaf_io;

        SpaceInspectStrategy qs;
        tree->queryStrategy(qs);

        cerr << "Indexed space: " << qs.m_indexedSpace << endl;
        cerr << "Operations: " << count << endl;
        cerr << *tree;
        cerr << "Index I/O: " << index_io << endl;
        cerr << "Leaf I/O: " << leaf_io << endl;
        cerr << "Buffer hits: " << buffer->getHits() << endl;

        delete tree;
        delete buffer;
        delete storage_manager;
    } catch (Tools::Exception& e) {
        cerr << "******ERROR******" << endl;
        std::string s = e.what();
        cerr << s << endl;
        return -1;
    } catch (...) {
        cerr << "******ERROR******" << endl;
        cerr << "Unknown exception" << endl;
        return -1;
    }
    return 0;
}
