#!/bin/bash

kubectl apply -f https://github.com/knative/serving/releases/download/knative-v1.15.2/serving-crds.yaml
kubectl apply -f https://github.com/knative/serving/releases/download/knative-v1.15.2/serving-core.yaml
kubectl apply -f https://github.com/knative/net-kourier/releases/download/knative-v1.15.1/kourier.yaml

# Patch the Knative config-network to use Kourier as the ingress class
kubectl patch configmap/config-network \
  --namespace knative-serving \
  --type merge \
  --patch '{"data":{"ingress-class":"kourier.ingress.networking.knative.dev"}}'

# Patch the Knative config-features to enable podspec-nodeselector
kubectl patch configmap/config-features \
  --namespace knative-serving \
  --type merge \
  --patch '{"data":{"kubernetes.podspec-nodeselector":"enabled"}}'
