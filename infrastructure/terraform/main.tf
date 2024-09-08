variable "project_id" {
  description = "The project name"
  type        = string
}

variable "cluster_name" {
  description = "The cluster name"
  type        = string
}

variable "region" {
  description = "The region"
  type        = string
  default     = "asia-northeast3"
}

terraform {
  required_version = "~> 1.9"

  required_providers {
    google = {
      source  = "hashicorp/google"
      version = "~> 5.42.0"
    }
  }
}

provider "google" {
  project = var.project_id
  region  = var.region
}

data "google_client_config" "default" {}

resource "google_container_cluster" "primary" {
  name     = var.cluster_name
  location = var.region

  network = "default"

  # Enabling Autopilot for this cluster
  enable_autopilot = true
}

output "cluster_name" {
  value = google_container_cluster.primary.name
}

output "attach_cmd" {
  value = "gcloud container clusters get-credentials ${google_container_cluster.primary.name} --region ${var.region} --project ${var.project_id}"
}
