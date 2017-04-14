FROM sbubmi/segmentation_base:latest

# Install-directories
ENV DIR=/tmp/pathomics_analysis/nucleusSegmentation
ENV BLD_DIR=$DIR/build
ENV SRC_DIR=$DIR/src

# Clone GitHub repository
RUN cd /tmp && git clone -b develop https://github.com/SBU-BMI/pathomics_analysis.git

# Build
RUN mkdir $BLD_DIR && cd $WORKDIR
WORKDIR $BLD_DIR
RUN cmake $SRC_DIR
RUN make -j4

# Copy executables
RUN cp app/main* /usr/local/bin/.

# Copy script
RUN cp ../script/mainAggregateFeatures.py  /usr/local/bin/.
