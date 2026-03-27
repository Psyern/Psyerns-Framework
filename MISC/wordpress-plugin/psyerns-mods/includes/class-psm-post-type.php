<?php
/**
 * Custom post type registration for Psyerns Mods.
 *
 * @package Psyerns_Mods
 */

if ( ! defined( 'ABSPATH' ) ) {
	exit;
}

/**
 * Registers the 'psyern_mod' custom post type.
 */
class PSM_Post_Type {

	/**
	 * Post type slug.
	 *
	 * @var string
	 */
	const SLUG = 'psyern_mod';

	/**
	 * Register the custom post type.
	 *
	 * Intended to be called on the 'init' action.
	 *
	 * @return void
	 */
	public static function register() {
		if ( post_type_exists( self::SLUG ) ) {
			return;
		}

		$labels = array(
			'name'                  => __( 'Mods', 'psyerns-mods' ),
			'singular_name'         => __( 'Mod', 'psyerns-mods' ),
			'add_new'               => __( 'Neue Mod hinzufügen', 'psyerns-mods' ),
			'add_new_item'          => __( 'Neue Mod hinzufügen', 'psyerns-mods' ),
			'edit_item'             => __( 'Mod bearbeiten', 'psyerns-mods' ),
			'new_item'              => __( 'Neue Mod', 'psyerns-mods' ),
			'view_item'             => __( 'Mod ansehen', 'psyerns-mods' ),
			'view_items'            => __( 'Mods ansehen', 'psyerns-mods' ),
			'search_items'          => __( 'Mods durchsuchen', 'psyerns-mods' ),
			'not_found'             => __( 'Keine Mods gefunden', 'psyerns-mods' ),
			'not_found_in_trash'    => __( 'Keine Mods im Papierkorb gefunden', 'psyerns-mods' ),
			'all_items'             => __( 'Alle Mods', 'psyerns-mods' ),
			'menu_name'             => __( 'Psyerns Mods', 'psyerns-mods' ),
			'name_admin_bar'        => __( 'Mod', 'psyerns-mods' ),
			'featured_image'        => __( 'Mod-Bild', 'psyerns-mods' ),
			'set_featured_image'    => __( 'Mod-Bild festlegen', 'psyerns-mods' ),
			'remove_featured_image' => __( 'Mod-Bild entfernen', 'psyerns-mods' ),
			'use_featured_image'    => __( 'Als Mod-Bild verwenden', 'psyerns-mods' ),
		);

		$args = array(
			'labels'              => $labels,
			'public'              => false,
			'publicly_queryable'  => false,
			'show_ui'             => true,
			'show_in_menu'        => true,
			'query_var'           => false,
			'rewrite'             => false,
			'capability_type'     => 'post',
			'has_archive'         => false,
			'hierarchical'        => false,
			'menu_position'       => 25,
			'menu_icon'           => 'dashicons-games',
			'supports'            => array( 'title', 'thumbnail' ),
			'show_in_rest'        => false,
		);

		register_post_type( self::SLUG, $args );
	}
}
