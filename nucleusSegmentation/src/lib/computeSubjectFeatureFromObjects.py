import numpy as np
import h5py

def computeMultipleSubjectFeatures(featureFilePathnameList, outputFilePathname):
    f = h5py.File(outputFilePathname, 'x')
    n = len(featureFilePathnameList)

    #--------------------------------------------------------------------------------
    # test to sea the dimension of feature of each subject
    featureFilePathname = featureFilePathnameList[0]
    v = computeSubjectFeatureFromObjects(featureFilePathname)
    dim = v.size

    dset = f.create_dataset('features', (n, dim), compression='gzip')

    for it in range(n):
        print('processing files ' + str(it) + ' / ' + str(n) )
        featureFilePathname = featureFilePathnameList[it]
        v = computeSubjectFeatureFromObjects(featureFilePathname)
        dset[it, :] = v

    f.close()

def computeSubjectFeatureFromObjects(featureFilePathname):
    f = open(featureFilePathname,'r')
    allContent = f.read().split('\n')
    f.close()

    print('There are ' + str(len(allContent)) + " objects.")

    nObjects = len(allContent) - 2 # first row are names, last row is empty

    # print(allContent[0])
    # print(allContent[1])

    names = allContent[0]
    names = names.split(',')
    names = names[:-1] # coz allContent[0] ends with ',', after split, the last one is an empty string

    numberOfFeatures = len(names) - 1 # coz the last one is "Polygon"

    # print(len(names))
    # print(names)



    allFeatureMatrix = np.empty([nObjects, numberOfFeatures], dtype=float) # float is the default, but i'm explicite to emphasize


    for iObject in range(nObjects):
        featuresOfThisObject = allContent[iObject + 1]
        featuresOfThisObject = featuresOfThisObject.split(',')
        featuresOfThisObject = featuresOfThisObject[:numberOfFeatures]
        featuresOfThisObject = [float(x) for x in featuresOfThisObject]

        allFeatureMatrix[iObject, :] = np.asarray(featuresOfThisObject)

        # print(len(featuresOfThisObject))
        # print(featuresOfThisObject)

        #featureMedian = np.median(allFeatureMatrix, axis=1)


    [featureQ25, featureQ75, featureMedian] = np.percentile(allFeatureMatrix, [25, 50, 75], axis=0)


    subjectFeatures = np.concatenate((featureQ25, featureQ75, featureMedian))

    return subjectFeatures
