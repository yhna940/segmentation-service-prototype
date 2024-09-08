<!-- BEGIN_TF_DOCS -->
## Requirements

| Name | Version |
|------|---------|
| <a name="requirement_terraform"></a> [terraform](#requirement\_terraform) | ~> 1.9 |
| <a name="requirement_google"></a> [google](#requirement\_google) | ~> 5.42.0 |

## Providers

| Name | Version |
|------|---------|
| <a name="provider_google"></a> [google](#provider\_google) | 5.42.0 |

## Modules

No modules.

## Resources

| Name | Type |
|------|------|
| [google_container_cluster.primary](https://registry.terraform.io/providers/hashicorp/google/latest/docs/resources/container_cluster) | resource |

## Inputs

| Name | Description | Type | Default | Required |
|------|-------------|------|---------|:--------:|
| <a name="input_cluster_name"></a> [cluster\_name](#input\_cluster\_name) | The cluster name | `string` | `"test-ecai"` | no |
| <a name="input_project_id"></a> [project\_id](#input\_project\_id) | The project name | `string` | `"lunit-ecai"` | no |
| <a name="input_region"></a> [region](#input\_region) | The region | `string` | `"asia-northeast3"` | no |

## Outputs

| Name | Description |
|------|-------------|
| <a name="output_attach_cmd"></a> [attach\_cmd](#output\_attach\_cmd) | n/a |
| <a name="output_cluster_name"></a> [cluster\_name](#output\_cluster\_name) | n/a |
<!-- END_TF_DOCS -->
