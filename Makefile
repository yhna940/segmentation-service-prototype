# Variables
PATCH_SERVER_IMAGE_NAME = lunityounghwan/remote-segmentor-patch-server
DISPATCHER_IMAGE_NAME = lunityounghwan/remote-segmentor-dispatcher
TAG ?= latest
PLATFORM ?= linux/amd64
TERRAFORM_DIR = ./infrastructure/terraform
HELM_NAMESPACE ?= default
HELM_RELEASE_NAME = remote-segmentor
HELM_CHART_PATH = ./helm

# Build patch-server Docker image with optional TAG argument
build-patch-server:
	docker build --platform=$(PLATFORM) -t $(PATCH_SERVER_IMAGE_NAME):$(TAG) ./patch-server

# Build dispatcher Docker image with optional TAG argument
build-dispatcher:
	docker build --platform=$(PLATFORM) -t $(DISPATCHER_IMAGE_NAME):$(TAG) ./dispatcher

# Push patch-server Docker image with optional TAG argument
push-patch-server:
	docker push $(PATCH_SERVER_IMAGE_NAME):$(TAG)

# Push dispatcher Docker image with optional TAG argument
push-dispatcher:
	docker push $(DISPATCHER_IMAGE_NAME):$(TAG)

# Initialize Terraform
terraform-init:
	cd $(TERRAFORM_DIR) && terraform init

# Plan Terraform changes
terraform-plan:
	cd $(TERRAFORM_DIR) && terraform plan

# Apply Terraform changes
terraform-apply:
	cd $(TERRAFORM_DIR) && terraform apply -auto-approve

# Destroy Terraform-managed infrastructure
terraform-destroy:
	cd $(TERRAFORM_DIR) && terraform destroy -auto-approve

# Run knative-setup.sh after applying Terraform
knative-setup:
	bash ./infrastructure/script/knative-setup.sh

# Helm install with namespace option
helm-install:
	helm install $(HELM_RELEASE_NAME) $(HELM_CHART_PATH) --namespace $(HELM_NAMESPACE) --create-namespace

# Helm upgrade or install with namespace option
helm-upgrade:
	helm upgrade --install $(HELM_RELEASE_NAME) $(HELM_CHART_PATH) --namespace $(HELM_NAMESPACE) --create-namespace

# Helm uninstall with namespace option
helm-uninstall:
	helm uninstall $(HELM_RELEASE_NAME) --namespace $(HELM_NAMESPACE)
