FROM sbubmi/segmentation_base:latest

# Install-directories
ENV DIR=/tmp/pathomics_analysis/nucleusSegmentation
ENV BLD_DIR=$DIR/build
ENV SRC_DIR=$DIR/src

RUN mkdir -p /tmp/pathomics_analysis
WORKDIR /tmp/pathomics_analysis
COPY . /tmp/pathomics_analysis

# Build
RUN mkdir $BLD_DIR && cd $WORKDIR
WORKDIR $BLD_DIR
RUN cmake $SRC_DIR && make -j4

# Copy executables
RUN mv app/main* /usr/local/bin/. && \
    mv app/computeFeatures* /usr/local/bin/. && \
    mv ../script/mainAggregateFeatures.py  /usr/local/bin/. 

ENV ITK_GLOBAL_DEFAULT_NUMBER_OF_THREADS=1

WORKDIR /tmp

CMD ["/bin/bash"]
