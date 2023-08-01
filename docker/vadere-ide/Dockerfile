# vadere Crowd Simulator
#

ARG IMAGE_REG
ARG BASE_IMAGE_TAG
ARG CROWNET_COMMIT_HASH

FROM ${IMAGE_REG}/crownet-base-java:${BASE_IMAGE_TAG}

# Specify the version/tag of vadere to be put in the container:
ENV RELEASE $CROWNET_COMMIT_HASH

LABEL description "vadere Crowd Simulator (as used in crownet)"
LABEL maintainer  "christina_maria.mayr@hm.edu"

# init script (triggered at container start)
COPY docker/vadere-ide/init.sh /init.sh
RUN chmod 555 /init.sh
ENTRYPOINT [ "/init.sh" ]

# waitfor script (in order to wait for non-child processes)
COPY docker/vadere-ide/waitfor.sh /waitfor.sh
RUN chmod 555 /waitfor.sh

# add ide (intellij)
WORKDIR /opt/intellij
RUN wget https://download.jetbrains.com/idea/ideaIU-2023.2.tar.gz --output-document /opt/intellij/intellij.tar.gz
RUN tar xzvf /opt/intellij/intellij.tar.gz --one-top-level=intellij --strip-components 1
ENV PATH="/opt/intellij/intellij/bin:${PATH}"



