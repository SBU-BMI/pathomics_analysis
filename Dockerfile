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
RUN cmake $SRC_DIR
RUN make -j4

# Copy executables
RUN cp app/main* /usr/local/bin/.

# Copy script
RUN cp ../script/mainAggregateFeatures.py  /usr/local/bin/.

