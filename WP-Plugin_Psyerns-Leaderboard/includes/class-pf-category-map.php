<?php
/**
 * Category Mapping / Pretty Names.
 *
 * Resolves raw DayZ classnames (e.g. "ZmbM_PriestPopSkinny", "Wolf_Grey",
 * "Boss_Frostbite", "Survivor_Boris") into human-readable display labels,
 * grouping buckets, and icon slugs for use by the player-detail modal.
 *
 * @package Psyerns_Framework
 */

if ( ! defined( 'ABSPATH' ) ) {
	exit;
}

/**
 * Class PF_Category_Map.
 *
 * Pure static utility — no instance state.
 *
 * Example:
 *   Input  → [ 'ZmbM_PriestPopSkinny' => 12, 'Wolf_Grey' => 3, 'Survivor_Boris' => 2 ]
 *   Output → [
 *     'zombies' => [ 'Priest'    => 12 ],
 *     'players' => [ 'Player'    => 2  ],
 *     'bosses'  => [],
 *     'ai'      => [],
 *     'animals' => [ 'Grey Wolf' => 3 ],
 *     'other'   => [],
 *   ]
 */
class PF_Category_Map {

	/**
	 * Vanilla zombie classname → pretty label lookup.
	 *
	 * @var array<string,string>
	 */
	private static $vanilla_zombies = array(
		'ZmbM_PolicemanFat'         => 'Police Officer',
		'ZmbM_Policeman'            => 'Police Officer',
		'ZmbM_PriestPopSkinny'      => 'Priest',
		'ZmbM_Priest'               => 'Priest',
		'ZmbF_JournalistNormal'     => 'Journalist',
		'ZmbF_JournalistNormal_Black' => 'Journalist',
		'ZmbM_HermitSkinny_Beige'   => 'Hermit',
		'ZmbM_HermitSkinny'         => 'Hermit',
		'ZmbM_SoldierNormal'        => 'Soldier',
		'ZmbM_PatrolNormal_PautRev' => 'Soldier',
		'ZmbM_ParamedicNormal'      => 'Paramedic',
		'ZmbM_DoctorFat'            => 'Doctor',
		'ZmbF_DoctorFat'            => 'Doctor',
		'ZmbM_FirefighterNormal'    => 'Firefighter',
		'ZmbM_NioshNormal'          => 'NIOSH Worker',
		'ZmbM_HunterOld_Autumn'     => 'Hunter',
		'ZmbF_HikerSkinny_Green'    => 'Hiker',
		'ZmbM_CitizenASkinny'       => 'Civilian',
		'ZmbF_CitizenAFat_Red'      => 'Civilian',
		'ZmbM_VillagerOld'          => 'Villager',
		'ZmbF_VillagerOld'          => 'Villager',
		'ZmbM_PrisonerSkinny'       => 'Prisoner',
		'ZmbM_ClerkFat_White'       => 'Clerk',
		'ZmbF_ClerkFat_White'       => 'Clerk',
		'ZmbM_MechanicSkinny'       => 'Mechanic',
		'Infected_Citizen'          => 'Civilian',
		'Infected_Soldier'          => 'Soldier',
	);

	/**
	 * Animal classname → pretty label lookup.
	 *
	 * @var array<string,string>
	 */
	private static $vanilla_animals = array(
		'Wolf_Grey'                  => 'Grey Wolf',
		'Wolf_Black'                 => 'Black Wolf',
		'Bear_Brown'                 => 'Brown Bear',
		'Animal_CapreolusCapreolusF' => 'Roe Doe',
		'Animal_CapreolusCapreolus'  => 'Roe Buck',
		'Animal_CervusElaphus'       => 'Red Deer',
		'Animal_CervusElaphusF'      => 'Red Hind',
		'Animal_GallusGallusDomesticus' => 'Chicken',
		'Animal_GallusGallusDomesticusF' => 'Hen',
		'Animal_SusDomesticus'       => 'Pig',
		'Animal_BosTaurus'           => 'Cow',
		'Animal_OvisAries'           => 'Sheep',
		'Animal_CapraHircus'         => 'Goat',
		'Animal_VulpesVulpes'        => 'Fox',
		'Animal_UrsusArctos'         => 'Brown Bear',
		'Animal_CanisLupus_Grey'     => 'Grey Wolf',
		'Animal_CanisLupus_White'    => 'White Wolf',
		'Animal_SusScrofa'           => 'Wild Boar',
	);

	/**
	 * Hardcoded boss classnames (filterable).
	 *
	 * @return array<int,string>
	 */
	private static function get_boss_classes() {
		$defaults = array(
			'Boss_Frostbite',
			'Boss_Necromancer',
			'Boss_Reanimated',
			'Boss_Inferno',
			'Boss_Outbreak',
		);
		return apply_filters( 'pf_category_boss_classes', $defaults );
	}

	/**
	 * Hardcoded AI classnames (filterable, exact match list).
	 *
	 * @return array<int,string>
	 */
	private static function get_ai_classes() {
		$defaults = array(
			'AI_Bandit',
			'AI_NPC_Guard',
			'NPC_Trader',
			'Trader',
		);
		return apply_filters( 'pf_category_ai_classes', $defaults );
	}

	/**
	 * Resolve the bucket group for a classname.
	 *
	 * Returns one of: zombies | players | bosses | ai | animals | other.
	 *
	 * @param string $classname Raw DayZ classname.
	 * @return string
	 */
	public static function get_group( string $classname ): string {
		$group = self::resolve_group( $classname );
		return apply_filters( 'pf_category_group', $group, $classname );
	}

	/**
	 * Internal group resolver (no filter applied).
	 *
	 * @param string $classname Raw DayZ classname.
	 * @return string
	 */
	private static function resolve_group( string $classname ): string {
		if ( '' === $classname ) {
			return 'other';
		}

		// 1) Bosses — prefix or explicit list.
		if ( preg_match( '/^Boss_/i', $classname ) ) {
			return 'bosses';
		}
		$bosses = self::get_boss_classes();
		foreach ( $bosses as $boss ) {
			if ( 0 === strcasecmp( $boss, $classname ) ) {
				return 'bosses';
			}
			// Allow prefix entries like 'Boss_Reanimated_*'.
			if ( preg_match( '/^' . preg_quote( $boss, '/' ) . '/i', $classname ) ) {
				return 'bosses';
			}
		}

		// 2) Players.
		$player_aliases = array( 'Player', 'Survivor', 'Players', 'Spieler' );
		foreach ( $player_aliases as $alias ) {
			if ( 0 === strcasecmp( $alias, $classname ) ) {
				return 'players';
			}
		}
		if ( preg_match( '/^Survivor/i', $classname ) ) {
			return 'players';
		}

		// 3) Zombies.
		if ( preg_match( '/^(Zmb[MF]_|ZombieM_|ZombieF_|Infected_)/i', $classname ) ) {
			return 'zombies';
		}

		// 5) AI — checked BEFORE animals so "AI_Bandit" etc. don't accidentally match animal patterns.
		$ai_classes = self::get_ai_classes();
		foreach ( $ai_classes as $ai ) {
			if ( 0 === strcasecmp( $ai, $classname ) ) {
				return 'ai';
			}
		}
		if ( preg_match( '/^(AI_|NPC_|Bandit|Trader|Soldier_|Guard_|Military_)/i', $classname ) ) {
			return 'ai';
		}

		// 4) Animals.
		if ( preg_match( '/^(Animal_|Wolf|Bear|Wild|Hen|Cow|Pig|Goat|Sheep|Deer|Fox|Boar|Roe|Mouflon|Chicken)/i', $classname ) ) {
			return 'animals';
		}

		// 6) Other.
		return 'other';
	}

	/**
	 * Resolve a display label for a classname.
	 *
	 * @param string $classname Raw DayZ classname.
	 * @return string
	 */
	public static function get_label( string $classname ): string {
		$label = self::resolve_label( $classname );
		return apply_filters( 'pf_category_label', $label, $classname );
	}

	/**
	 * Internal label resolver.
	 *
	 * @param string $classname Raw DayZ classname.
	 * @return string
	 */
	private static function resolve_label( string $classname ): string {
		if ( '' === $classname ) {
			return '';
		}

		$group = self::resolve_group( $classname );

		switch ( $group ) {
			case 'players':
				return 'Player';

			case 'bosses':
				// "Boss_Frostbite" → "Frostbite Boss", "Boss_Reanimated_Foo" → "Reanimated Foo Boss".
				$stripped = preg_replace( '/^Boss_/i', '', $classname );
				if ( $stripped === $classname || '' === $stripped ) {
					return $classname;
				}
				$pretty = self::prettify( $stripped );
				return $pretty . ' Boss';

			case 'zombies':
				if ( isset( self::$vanilla_zombies[ $classname ] ) ) {
					return self::$vanilla_zombies[ $classname ];
				}
				$stripped = preg_replace( '/^(Zmb[MF]_|ZombieM_|ZombieF_|Infected_)/i', '', $classname );
				return self::prettify( $stripped );

			case 'animals':
				if ( isset( self::$vanilla_animals[ $classname ] ) ) {
					return self::$vanilla_animals[ $classname ];
				}
				$stripped = preg_replace( '/^Animal_/i', '', $classname );
				return self::prettify( $stripped );

			case 'ai':
				$stripped = preg_replace( '/^(AI_NPC_|AI_|NPC_)/i', '', $classname );
				if ( '' === $stripped ) {
					return $classname;
				}
				return self::prettify( $stripped );

			case 'other':
			default:
				return $classname;
		}
	}

	/**
	 * Resolve an icon slug for the classname.
	 *
	 * @param string $classname Raw DayZ classname.
	 * @return string
	 */
	public static function get_icon( string $classname ): string {
		$group = self::resolve_group( $classname );
		switch ( $group ) {
			case 'zombies':
				return 'zombie';
			case 'players':
				return 'player';
			case 'bosses':
				return 'boss';
			case 'ai':
				return 'ai';
			case 'animals':
				if ( preg_match( '/Wolf/i', $classname ) || preg_match( '/CanisLupus/i', $classname ) ) {
					return 'wolf';
				}
				if ( preg_match( '/Bear/i', $classname ) || preg_match( '/Ursus/i', $classname ) ) {
					return 'bear';
				}
				return 'animal';
			default:
				return '';
		}
	}

	/**
	 * Bucket a raw category-kills (or deaths) map into the six standard groups.
	 *
	 * Always returns all six bucket keys, even if empty. Duplicate labels
	 * within the same bucket are summed defensively.
	 *
	 * @param array $raw Map of classname => int count.
	 * @return array{
	 *   zombies: array<string,int>,
	 *   players: array<string,int>,
	 *   bosses:  array<string,int>,
	 *   ai:      array<string,int>,
	 *   animals: array<string,int>,
	 *   other:   array<string,int>
	 * }
	 */
	public static function bucket_kills( array $raw ): array {
		$buckets = array(
			'zombies' => array(),
			'players' => array(),
			'bosses'  => array(),
			'ai'      => array(),
			'animals' => array(),
			'other'   => array(),
		);

		foreach ( $raw as $classname => $count ) {
			$classname = (string) $classname;
			$count     = (int) $count;
			if ( '' === $classname ) {
				continue;
			}

			$group = self::get_group( $classname );
			$label = self::get_label( $classname );

			if ( ! isset( $buckets[ $group ] ) ) {
				$group = 'other';
			}

			if ( isset( $buckets[ $group ][ $label ] ) ) {
				$buckets[ $group ][ $label ] += $count;
			} else {
				$buckets[ $group ][ $label ] = $count;
			}
		}

		return $buckets;
	}

	/**
	 * Convert a CamelCase / snake_case / mixed token into a Title Case label.
	 *
	 * "PriestPopSkinny"        → "Priest Pop Skinny"
	 * "CitizenAFat_Red"        → "Citizen A Fat Red"
	 * "police_officer"         → "Police Officer"
	 *
	 * @param string $token Stripped classname segment.
	 * @return string
	 */
	private static function prettify( string $token ): string {
		if ( '' === $token ) {
			return '';
		}
		// Replace underscores with spaces.
		$out = str_replace( '_', ' ', $token );
		// Insert space before uppercase letters that follow lowercase / digit.
		$out = preg_replace( '/([a-z0-9])([A-Z])/', '$1 $2', $out );
		// Collapse multiple spaces.
		$out = preg_replace( '/\s+/', ' ', $out );
		$out = trim( $out );
		// Title-case.
		return ucwords( strtolower( $out ) );
	}
}
